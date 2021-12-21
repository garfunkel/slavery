#define _POSIX_C_SOURCE 200809L

#include "libslavery.h"

#include "libslavery_p.h"

#include <fcntl.h>
#include <libudev.h>
#include <linux/hidraw.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <threads.h>
#include <unistd.h>

const uint16_t SLAVERY_USB_VENDOR_ID_LOGITECH = 0x046d;
const uint16_t SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER = 0xc52b;

const ssize_t SLAVERY_HIDPP_PACKET_LENGTH_SHORT = 7;
const ssize_t SLAVERY_HIDPP_PACKET_LENGTH_LONG = 20;

const uint8_t SLAVERY_HIDPP_SOFTWARE_ID = 0x01;
const uint8_t SLAVERY_HIDPP_FEATURE_ROOT = 0x00;

void slavery_receiver_array_free(slavery_receiver_t *receivers[], const ssize_t num_receivers) {
	for (int i = 0; i < num_receivers; i++) {
		slavery_receiver_free(receivers[i]);
	}

	free(receivers);
}

void slavery_receiver_array_print(const slavery_receiver_t *receivers[], const ssize_t num_receivers) {
	printf("Receivers: %lu\n\n", num_receivers);

	for (int i = 0; i < num_receivers; i++) {
		printf("Receiver %d:\n", i + 1);
		slavery_receiver_print(receivers[i]);

		if (i < num_receivers - 1) {
			puts("\n");
		}
	}
}

// TODO: free devices and listeners.
void slavery_receiver_free(slavery_receiver_t *receiver) {
	close(receiver->fd);

	free(receiver->devnode);
	free(receiver->name);
	free(receiver->address);
	free(receiver);
}

void slavery_receiver_print(const slavery_receiver_t *receiver) {
	printf("Devnode: %s\n", receiver->devnode);
	printf("Vendor ID: %04hx\n", receiver->vendor_id);
	printf("Product ID: %04hx\n", receiver->product_id);
}

int slavery_scan_receivers(slavery_receiver_t **receivers[]) {
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *device_list, *device_entry;
	int num_receivers = 0;
	*receivers = NULL;

	if ((udev = udev_new()) == NULL) {
		perror("udev_new()");

		return -1;
	}

	if ((enumerate = udev_enumerate_new(udev)) == NULL) {
		perror("udev_enumeate_new()");

		udev_unref(udev);

		return -1;
	}

	if (udev_enumerate_add_match_subsystem(enumerate, "hidraw") < 0) {
		perror("udev_enumerate_add_match_subsystem()");

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		return -1;
	}

	if (udev_enumerate_scan_devices(enumerate) < 0) {
		perror("udev_enumerate_scan_devices()");

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		return -1;
	}

	device_list = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(device_entry, device_list) {
		const char *sys_path = udev_list_entry_get_name(device_entry);
		struct udev_device *device = udev_device_new_from_syspath(udev, sys_path);
		struct udev_device *parent_device = device;
		const char *devnode = udev_device_get_devnode(device);
		slavery_receiver_t *receiver = slavery_receiver_from_devnode(devnode);

		if (receiver == NULL) {
			udev_device_unref(device);

			continue;
		}

		slavery_receiver_get_report_descriptor(receiver);

		*receivers = realloc(*receivers, sizeof(slavery_receiver_t) * (num_receivers + 1));
		*receivers[num_receivers++] = receiver;

		udev_device_unref(device);
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);

	return num_receivers;
}

slavery_receiver_t *slavery_receiver_from_devnode(const char *devnode) {
	slavery_receiver_t *receiver = malloc(sizeof(slavery_receiver_t));
	struct hidraw_devinfo info;
	int length;

	if ((receiver->fd = open(devnode, O_RDWR)) < 0) {
		perror("open()");

		free(receiver);

		return NULL;
	}

	if ((ioctl(receiver->fd, HIDIOCGRAWINFO, &info)) < 0) {
		perror("ioctl(HIDIOCGRAWINFO)");

		close(receiver->fd);
		free(receiver);

		return NULL;
	}

	if ((uint16_t)info.vendor != SLAVERY_USB_VENDOR_ID_LOGITECH ||
	    (uint16_t)info.product != SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER) {
		close(receiver->fd);
		free(receiver);

		return NULL;
	}

	receiver->devnode = strdup(devnode);
	receiver->vendor_id = info.vendor;
	receiver->product_id = info.product;
	receiver->name = malloc(256);

	if ((length = ioctl(receiver->fd, HIDIOCGRAWNAME(256), receiver->name)) < 0) {
		perror("ioctl(HIDIOCGRAWNAME)");

		close(receiver->fd);
		free(receiver->name);
		free(receiver);

		return NULL;
	}

	receiver->name = realloc(receiver->name, length);
	receiver->address = malloc(256);

	if ((length = ioctl(receiver->fd, HIDIOCGRAWPHYS(256), receiver->address)) < 0) {
		perror("ioctl(HIDIOCGRAWPHYS)");

		close(receiver->fd);
		free(receiver->name);
		free(receiver->address);
		free(receiver);

		return NULL;
	}

	receiver->address = realloc(receiver->address, length);
	receiver->num_devices = 0;

	if (mtx_init(&receiver->listener_lock, mtx_plain) != thrd_success) {
		close(receiver->fd);
		free(receiver->name);
		free(receiver->address);
		free(receiver);

		return NULL;
	}

	return receiver;
}

int slavery_receiver_get_report_descriptor(slavery_receiver_t *receiver) {
	struct hidraw_report_descriptor desc;

	if ((ioctl(receiver->fd, HIDIOCGRDESCSIZE, &desc.size)) < 0) {
		perror("ioctl(HIDIOCGRDESCSIZE)");

		return -1;
	}

	if ((ioctl(receiver->fd, HIDIOCGRDESC, &desc)) < 0) {
		perror("ioctl(HIDIOCGRDESC)");

		return -1;
	}

	/*printf("Report Descriptor:\n");
	for (unsigned int i = 0; i < desc.size; i++)
	    printf("%hhx ", desc.value[i]);
	puts("\n");*/

	return 0;
}

int slavery_receiver_get_devices(slavery_receiver_t *receiver, slavery_device_t **devices[]) {
	int num_devices = 0;
	*devices = NULL;

	for (uint8_t device_index = SLAVERY_HIDPP_DEVICE_INDEX_1; device_index <= SLAVERY_HIDPP_DEVICE_INDEX_6;
	     device_index++) {
		slavery_device_t *device = slavery_device_from_receiver(receiver, device_index);

		if (device == NULL) {
			continue;
		}

		*devices = realloc(*devices, sizeof(slavery_device_t) * (num_devices + 1));
		*devices[num_devices++] = device;

		slavery_device_print(device);
	}

	return num_devices;
}

int slavery_receiver_listen(slavery_receiver_t *receiver) {
	struct pollfd poll_fds[] = {
	    {.fd = receiver->listener_fd, .events = POLLIN, .revents = POLLIN | POLLERR | POLLHUP | POLLNVAL},
	    {.fd = receiver->listener_pipe[0],
	     .events = POLLIN,
	     .revents = POLLIN | POLLERR | POLLHUP | POLLNVAL}};
	int n;

	while (TRUE) {
		if ((n = poll(poll_fds, 2, -1)) > 0) {
			if (poll_fds[0].revents & POLLHUP) {
				printf("thread %ld hangup...\n", thrd_current());

				break;
			}

			if (poll_fds[0].revents & POLLERR) {
				printf("thread %ld error...\n", thrd_current());

				break;
			}

			if (poll_fds[0].revents & POLLNVAL) {
				printf("thread %ld nval...\n", thrd_current());

				break;
			}

			if (poll_fds[0].revents & POLLIN) {
				char buf[256];

				mtx_lock(&receiver->listener_lock);

				read(receiver->listener_fd, buf, 256);

				mtx_unlock(&receiver->listener_lock);

				if (n < 0) {
					break;
				}
			}
		}
	}

	return 0;
}

int slavery_receiver_start_listener(slavery_receiver_t *receiver) {
	if (pipe(receiver->listener_pipe) < 0) {
		return -1;
	}

	if ((receiver->listener_fd = dup(receiver->fd)) < 0) {
		return -1;
	}

	if (thrd_create(&receiver->listener_thread, (int (*)(void *))slavery_receiver_listen, receiver) !=
	    thrd_success) {
		return -1;
	}

	return 0;
}

int slavery_receiver_stop_listener(slavery_receiver_t *receiver) {
	int ret;

	if (close(receiver->listener_fd) < 0) {
		return -1;
	}

	if (write(receiver->listener_pipe[1], (uint8_t[]){1}, sizeof(uint8_t)) < 0) {
		return -1;
	}

	if (thrd_join(receiver->listener_thread, &ret) != thrd_success) {
		return -1;
	}

	if (close(receiver->listener_pipe[0]) < 0) {
		return -1;
	}

	if (close(receiver->listener_pipe[1]) < 0) {
		return -1;
	}

	return 0;
}

void slavery_device_array_free(slavery_device_t *devices[], const ssize_t num_devices) {
	for (ssize_t i = 0; i < num_devices; i++) {
		slavery_device_free(devices[i]);
	}

	free(devices);
}

slavery_device_t *slavery_device_from_receiver(slavery_receiver_t *receiver, const uint8_t device_index) {
	slavery_device_t *device = malloc(sizeof(slavery_device_t));
	device->receiver = receiver;
	device->index = device_index;

	if (slavery_hidpp_get_protocol_version(device) == NULL) {
		free(device);

		return NULL;
	}

	if (slavery_hidpp_get_name(device) == NULL) {
		free(device->protocol_version);
		free(device);

		return NULL;
	}

	if (slavery_hidpp_controls_get_num_buttons(device) == 0) {
		free(device->protocol_version);
		free(device->name);
		free(device);

		return NULL;
	}

	device->buttons = malloc(device->num_buttons * sizeof(slavery_button_t *));

	for (uint8_t i = 0; i < device->num_buttons; i++) {
		device->buttons[i] = slavery_hidpp_controls_get_button(device, i);

		if (device->buttons[i] == NULL) {
			free(device->buttons);
			free(device->protocol_version);
			free(device->name);
			free(device);

			return NULL;
		}

		if (device->buttons[i]->cid == 0x00c3) {
			slavery_hidpp_controls_button_remap(device->buttons[i]);
		}
	}

	return device;
}

void slavery_device_free(slavery_device_t *device) {
	free(device->protocol_version);
	free(device->name);

	for (uint8_t i = 0; i < device->num_buttons; i++) {
		free(device->buttons[i]);
	}

	free(device->buttons);
	free(device);
}

int slavery_device_set_config(slavery_device_t *device, const slavery_config_t *config) {
	return 0;
}

void slavery_device_print(const slavery_device_t *device) {
	printf("Device %d:\n", device->index);
	printf("Name: %s\n", device->name);
	printf("Protocol Version: %s\n", device->protocol_version);
	puts("\n");
}

uint8_t slavery_hidpp_lookup_feature_id(const slavery_device_t *device, const uint16_t number) {
	ssize_t n;
	uint8_t request_buf[] = {SLAVERY_REPORT_ID_SHORT,
	                         device->index,
	                         SLAVERY_HIDPP_FUNCTION_ROOT_GET_FEATURE,
	                         slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_ROOT_GET_FEATURE),
	                         number >> 8,
	                         number & 0xff,
	                         0x00};
	uint8_t response_buf[SLAVERY_HIDPP_PACKET_LENGTH_LONG];

	mtx_lock(&device->receiver->listener_lock);

	if ((n = write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT)) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		mtx_unlock(&device->receiver->listener_lock);

		return 0;
	}

	if ((n = read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG)) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		mtx_unlock(&device->receiver->listener_lock);

		return 0;
	}

	mtx_unlock(&device->receiver->listener_lock);

	if (response_buf[2] == 0x8f) {
		return 0;
	}

	return response_buf[4];
}

const char *slavery_hidpp_get_protocol_version(slavery_device_t *device) {
	uint8_t request_buf[] = {SLAVERY_REPORT_ID_SHORT,
	                         device->index,
	                         SLAVERY_HIDPP_FEATURE_ROOT,
	                         slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_ROOT_GET_PROTOCOL_VERSION),
	                         0x00,
	                         0x00,
	                         0x00};
	uint8_t response_buf[SLAVERY_HIDPP_PACKET_LENGTH_LONG];

	mtx_lock(&device->receiver->listener_lock);

	if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		mtx_unlock(&device->receiver->listener_lock);

		return NULL;
	}

	if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		mtx_unlock(&device->receiver->listener_lock);

		return NULL;
	}

	mtx_unlock(&device->receiver->listener_lock);

	if (response_buf[2] == 0x8f) {
		return NULL;
	}

	device->protocol_version = malloc(4);

	snprintf(device->protocol_version, 4, "%d.%d", response_buf[4], response_buf[5]);

	return device->protocol_version;
}

const char *slavery_hidpp_get_name(slavery_device_t *device) {
	uint8_t request_buf[] = {SLAVERY_REPORT_ID_SHORT,
	                         device->index,
	                         slavery_hidpp_lookup_feature_id(device, SLAVERY_HIDPP_ENTRY_POINT_NAME),
	                         slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_NAME_GET_LENGTH),
	                         0x00,
	                         0x00,
	                         0x00};
	uint8_t response_buf[SLAVERY_HIDPP_PACKET_LENGTH_LONG];

	mtx_lock(&device->receiver->listener_lock);

	if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		mtx_unlock(&device->receiver->listener_lock);

		return NULL;
	}

	if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		mtx_unlock(&device->receiver->listener_lock);

		return NULL;
	}

	mtx_unlock(&device->receiver->listener_lock);

	if (response_buf[2] == 0x8f) {
		return NULL;
	}

	uint8_t name_length = response_buf[4];
	char *name = malloc(name_length + 1);
	char *name_pos = name;
	request_buf[3] = slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_NAME_GET_NAME);

	printf("name len: %d\n", name_length + 1);

	do {
		mtx_lock(&device->receiver->listener_lock);

		if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
		    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
			mtx_unlock(&device->receiver->listener_lock);

			return NULL;
		}

		if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
		    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
			mtx_unlock(&device->receiver->listener_lock);

			return NULL;
		}

		mtx_unlock(&device->receiver->listener_lock);

		if (response_buf[2] == 0x8f) {
			return NULL;
		}

		ssize_t num_bytes = (name + name_length + 1) - name_pos;

		if (num_bytes > 16) {
			num_bytes = 16;
		}

		name_pos = stpncpy(name_pos, (const char *)response_buf + 4, num_bytes);
		request_buf[4] += num_bytes;
	} while (request_buf[4] < name_length);

	device->name = name;

	return device->name;
}

uint8_t slavery_hidpp_controls_get_num_buttons(slavery_device_t *device) {
	uint8_t request_buf[] = {SLAVERY_REPORT_ID_SHORT,
	                         device->index,
	                         slavery_hidpp_lookup_feature_id(device, SLAVERY_HIDPP_ENTRY_POINT_CONTROLS_V4),
	                         slavery_hidpp_encode_function(0),
	                         0x00,
	                         0x00,
	                         0x00};
	uint8_t response_buf[SLAVERY_HIDPP_PACKET_LENGTH_LONG];

	mtx_lock(&device->receiver->listener_lock);

	if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		mtx_unlock(&device->receiver->listener_lock);

		return 0;
	}

	if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		mtx_unlock(&device->receiver->listener_lock);

		return 0;
	}

	mtx_unlock(&device->receiver->listener_lock);

	if (response_buf[2] == 0x8f) {
		return 0;
	}

	device->num_buttons = response_buf[4];

	return device->num_buttons;
}

slavery_button_t *slavery_hidpp_controls_get_button(slavery_device_t *device, uint8_t button_index) {
	uint8_t request_buf[] = {SLAVERY_REPORT_ID_SHORT,
	                         device->index,
	                         slavery_hidpp_lookup_feature_id(device, SLAVERY_HIDPP_ENTRY_POINT_CONTROLS_V4),
	                         slavery_hidpp_encode_function(1),
	                         button_index,
	                         0x00,
	                         0x00};
	uint8_t response_buf[SLAVERY_HIDPP_PACKET_LENGTH_LONG];
	slavery_button_t *button;

	mtx_lock(&device->receiver->listener_lock);

	if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		mtx_unlock(&device->receiver->listener_lock);

		return NULL;
	}

	if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		mtx_unlock(&device->receiver->listener_lock);

		return NULL;
	}

	mtx_unlock(&device->receiver->listener_lock);

	if (response_buf[2] == 0x8f) {
		return NULL;
	}

	button = malloc(sizeof(slavery_button_t));

	printf("get button info: ");

	for (int i = 0; i < SLAVERY_HIDPP_PACKET_LENGTH_LONG; i++) {
		printf("%hhx ", response_buf[i]);
	}

	puts("");

	button->index = button_index;
	button->device = device;
	button->cid = (uint16_t)response_buf[4] << 8 | response_buf[5];
	button->task_id = (uint16_t)response_buf[6] << 8 | response_buf[7];
	button->flags = response_buf[8];
	button->virtual = response_buf[8] & 0x80;
	button->persistent_divert = response_buf[8] & 0x40;
	button->temporary_divert = response_buf[8] & 0x20;
	button->reprogrammable = response_buf[8] & 0x10;
	button->type = button->flags & 0x0f;
	button->function_position = response_buf[9];
	button->group = response_buf[10];
	button->group_remap_mask = response_buf[11];
	button->gesture = response_buf[12];

	printf("virtual: %d\n", button->virtual);
	printf("persistent divert: %d\n", button->persistent_divert);
	printf("temporary divert: %d\n", button->temporary_divert);
	printf("reprogrammable: %d\n", button->reprogrammable);
	printf("fn toggle: %d\n", button->type == SLAVERY_HIDPP_BUTTON_TYPE_FUNCTION_TOGGLE);
	printf("hotkey: %d\n", button->type == SLAVERY_HIDPP_BUTTON_TYPE_HOTKEY);
	printf("fn: %d\n", button->type == SLAVERY_HIDPP_BUTTON_TYPE_FUNCTION);
	printf("mouse: %d\n", button->type == SLAVERY_HIDPP_BUTTON_TYPE_BUTTON);
	printf("fn positon: %d\n", button->function_position);
	printf("group: %d\n", button->group);
	printf("group remap mask: %d\n", button->group_remap_mask);
	printf("gesture: %d\n", button->gesture);
	puts("");

	device->buttons[button_index] = button;

	return device->buttons[button_index];
}

/*
none = 0x33
*/
void slavery_hidpp_controls_button_remap(slavery_button_t *button) {
	uint8_t request_buf[] = {
	    SLAVERY_REPORT_ID_LONG,
	    button->device->index,
	    slavery_hidpp_lookup_feature_id(button->device, SLAVERY_HIDPP_ENTRY_POINT_CONTROLS_V4),
	    slavery_hidpp_encode_function(3),
	    button->cid >> 8,
	    button->cid & 0xff,
	    0x03,
	    button->cid >> 8,
	    button->cid & 0xff,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00,
	    0x00};
	uint8_t response_buf[SLAVERY_HIDPP_PACKET_LENGTH_LONG];

	printf("set button remap: ");

	for (int i = 0; i < SLAVERY_HIDPP_PACKET_LENGTH_LONG; i++) {
		printf("%hhx ", request_buf[i]);
	}

	puts("");

	mtx_lock(&button->device->receiver->listener_lock);

	if (write(button->device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		mtx_unlock(&button->device->receiver->listener_lock);

		return;
	}

	if (read(button->device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		mtx_unlock(&button->device->receiver->listener_lock);

		return;
	}

	mtx_unlock(&button->device->receiver->listener_lock);

	if (response_buf[2] == 0x8f) {
		return;
	}

	for (int i = 0; i < SLAVERY_HIDPP_PACKET_LENGTH_LONG; i++) {
		printf("%hhx ", response_buf[i]);
	}

	puts("");
}