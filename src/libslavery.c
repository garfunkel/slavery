#define _POSIX_C_SOURCE 200809L

#include "libslavery.h"

#include "libslavery_p.h"

#include <fcntl.h>
#include <libudev.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const char *SLAVERY_USB_VENDOR_ID_LOGITECH = "046d";
const char *SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER = "c52b";
const char *SLAVERY_USB_DRIVER_RECEIVER = "logitech-djreceiver";

const uint8_t SLAVERY_HIDPP_PACKET_LENGTH_SHORT = 7;
const uint8_t SLAVERY_HIDPP_PACKET_LENGTH_LONG = 20;

const uint8_t SLAVERY_HIDPP_SOFTWARE_ID = 0x01;
const uint8_t SLAVERY_HIDPP_FEATURE_ROOT = 0x00;

#define slavery_hidpp_encode_function(function) (function << 4) | SLAVERY_HIDPP_SOFTWARE_ID

slavery_receiver_list_t *slavery_receiver_list_new() {
	slavery_receiver_list_t *receiver_list = malloc(sizeof(slavery_receiver_list_t));

	memset(receiver_list, 0, sizeof(slavery_receiver_list_t));

	return receiver_list;
}

void slavery_receiver_list_free(slavery_receiver_list_t *receiver_list) {
	while (receiver_list) {
		slavery_receiver_free(receiver_list->receiver);

		slavery_receiver_list_t *old_receiver_list = receiver_list;
		receiver_list = receiver_list->next;

		free(old_receiver_list);
	}
}

void slavery_receiver_list_append(slavery_receiver_list_t *receiver_list, slavery_receiver_t *receiver) {
	if (receiver_list->receiver == NULL) {
		receiver_list->receiver = receiver;
		receiver_list->next = NULL;

		return;
	}

	while (receiver_list->next != NULL) {
		receiver_list = receiver_list->next;
	}

	receiver_list->next = malloc(sizeof(slavery_receiver_list_t));
	receiver_list->next->receiver = receiver;
	receiver_list->next->next = NULL;
}

ssize_t slavery_receiver_list_length(const slavery_receiver_list_t *receiver_list) {
	ssize_t length = 0;

	while (receiver_list) {
		length++;
		receiver_list = receiver_list->next;
	}

	return length;
}

void slavery_receiver_list_print(const slavery_receiver_list_t *receiver_list) {
	printf("Receivers: %lu\n\n", slavery_receiver_list_length(receiver_list));

	for (int receiver_number = 1; receiver_list; receiver_list = receiver_list->next, receiver_number++) {
		printf("Receiver %d:\n", receiver_number);
		slavery_receiver_print(receiver_list->receiver);

		if (receiver_list->next) {
			puts("\n");
		}
	}
}

void slavery_receiver_free(slavery_receiver_t *receiver) {
	free(receiver->devnode);
	free(receiver->vendor_id);
	free(receiver->product_id);
	free(receiver->driver);
	free(receiver);
}

void slavery_receiver_print(const slavery_receiver_t *receiver) {
	printf("Devnode: %s\n", receiver->devnode);
	printf("Vendor ID: %s\n", receiver->vendor_id);
	printf("Product ID: %s\n", receiver->product_id);
	printf("Driver: %s\n", receiver->driver);
}

slavery_receiver_list_t *slavery_scan_receivers() {
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *device_list, *device_entry;
	slavery_receiver_list_t *receiver_list = NULL;

	if ((udev = udev_new()) == NULL) {
		perror("udev_new()");

		return NULL;
	}

	if ((enumerate = udev_enumerate_new(udev)) == NULL) {
		perror("udev_enumeate_new()");

		udev_unref(udev);

		return NULL;
	}

	if (udev_enumerate_add_match_subsystem(enumerate, "hidraw") < 0) {
		perror("udev_enumerate_add_match_subsystem()");

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		return NULL;
	}

	if (udev_enumerate_scan_devices(enumerate) < 0) {
		perror("udev_enumerate_scan_devices()");

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		return NULL;
	}

	device_list = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(device_entry, device_list) {
		const char *sys_path = udev_list_entry_get_name(device_entry);
		struct udev_device *device = udev_device_new_from_syspath(udev, sys_path);
		struct udev_device *parent_device = device;
		const char *devnode = udev_device_get_devnode(device);
		const char *vendor_id = NULL;
		const char *product_id = NULL;
		const char *driver = NULL;

		while (parent_device != NULL) {
			if (vendor_id == NULL) {
				vendor_id = udev_device_get_sysattr_value(parent_device, "idVendor");
			}

			if (product_id == NULL) {
				product_id = udev_device_get_sysattr_value(parent_device, "idProduct");
			}

			if (driver == NULL) {
				driver = udev_device_get_sysattr_value(parent_device, "driver");
			}

			if (vendor_id && product_id && driver) {
				break;
			}

			parent_device = udev_device_get_parent(parent_device);
		}

		if (strcmp(vendor_id, SLAVERY_USB_VENDOR_ID_LOGITECH) != 0 ||
		    strcmp(product_id, SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER) != 0 ||
		    strcmp(driver, SLAVERY_USB_DRIVER_RECEIVER) != 0) {
			udev_device_unref(device);

			continue;
		}

		/*struct udev_list_entry *property_list =
		udev_device_get_sysattr_list_entry(device); struct udev_list_entry
		*property_entry;

		printf("properties:\n");

		udev_list_entry_foreach(property_entry, property_list) {
		    const char *property_name =
		udev_list_entry_get_name(property_entry); const char *property_value =
		udev_device_get_sysattr_value(device, "driver");
		    // const char *property_value =
		udev_list_entry_get_value(property_entry);

		    printf("\t%s: %s\n", property_name, property_value);
		}*/

		slavery_receiver_t *receiver = malloc(sizeof(slavery_receiver_t));
		receiver->devnode = strdup(devnode);
		receiver->vendor_id = strdup(vendor_id);
		receiver->product_id = strdup(product_id);
		receiver->driver = strdup(driver);
		receiver->fd = open(receiver->devnode, O_RDWR);

		if (receiver->fd < 0) {
			perror("open()");

			slavery_receiver_free(receiver);

			udev_device_unref(device);
			udev_enumerate_unref(enumerate);
			udev_unref(udev);

			return NULL;
		}

		if (receiver_list == NULL) {
			receiver_list = slavery_receiver_list_new();
		}

		slavery_receiver_list_append(receiver_list, receiver);
		udev_device_unref(device);
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);

	return receiver_list;
}

slavery_device_list_t *slavery_receiver_get_devices(slavery_receiver_t *receiver) {
	/*while (TRUE) {
	    ssize_t n;
	    char in_buf[256];

	    n = read(receiver->fd, in_buf, 256);

	    printf("n: %ld\t", n);

	    for (int i = 0; i < n; i++) {
	        printf("%hhx ", in_buf[i]);
	    }

	    puts("");
	}*/

	// for (uint8_t device_index = 0x40; device_index < 0x46; device_index++) {
	for (uint8_t device_index = SLAVERY_HIDPP_DEVICE_INDEX_1; device_index <= SLAVERY_HIDPP_DEVICE_INDEX_6;
	     device_index++) {
		slavery_device_t *device = slavery_device_from_receiver(receiver, device_index);

		if (device == NULL) {
			continue;
		}

		slavery_device_print(device);
	}

	return NULL;
}

void slavery_device_list_free(slavery_device_list_t *device_list) {
	while (device_list) {
		slavery_device_free(device_list->device);

		slavery_device_list_t *old_device_list = device_list;
		device_list = device_list->next;

		free(old_device_list);
	}
}

slavery_device_t *slavery_device_from_receiver(slavery_receiver_t *receiver, const uint8_t device_index) {
	// TODO: constructor instead as members are uninitialised => error when slavery_device_free/free()
	slavery_device_t *device = malloc(sizeof(slavery_device_t));
	device->receiver = receiver;
	device->index = device_index;

	if (slavery_hidpp_get_protocol_version(device) == NULL) {
		free(device);

		return NULL;
	}

	if (slavery_hidpp_get_name(device) == NULL) {
		slavery_device_free(device);

		return NULL;
	}

	if (slavery_hidpp_controls_get_num_buttons(device) == 0) {
		slavery_device_free(device);

		return NULL;
	}

	device->buttons = calloc(device->num_buttons, sizeof(slavery_button_t *));

	for (uint8_t i = 0; i < device->num_buttons; i++) {
		device->buttons[i] = slavery_hidpp_controls_get_button(device, i);

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

int slavery_device_set_config(slavery_device_t *device, const slavery_config_t *config) { return 0; }

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

	if ((n = write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT)) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		return 0;
	}

	if ((n = read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG)) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		return 0;
	}

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

	if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		return NULL;
	}

	if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		return NULL;
	}

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

	if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		return NULL;
	}

	if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		return NULL;
	}

	if (response_buf[2] == 0x8f) {
		return NULL;
	}

	uint8_t name_length = response_buf[4];
	char *name = malloc(name_length + 1);
	request_buf[3] = slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_NAME_GET_NAME);

	do {
		if (write(device->receiver->fd, request_buf, 7) != 7) {
			return NULL;
		}

		if (read(device->receiver->fd, response_buf, 20) != 20) {
			return NULL;
		}

		if (response_buf[2] == 0x8f) {
			return NULL;
		}

		strncpy(name + request_buf[4], (const char *)response_buf + 4, 16);

		request_buf[4] += 16;
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

	if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		return 0;
	}

	if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		return 0;
	}

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

	if (write(device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_SHORT) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_SHORT) {
		return NULL;
	}

	if (read(device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		return NULL;
	}

	if (response_buf[2] == 0x8f) {
		return NULL;
	}

	button = malloc(sizeof(slavery_button_t));

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

	for (int i = 0; i < SLAVERY_HIDPP_PACKET_LENGTH_LONG; i++) {
		printf("%hhx ", request_buf[i]);
	}

	puts("");

	if (write(button->device->receiver->fd, request_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		return;
	}

	if (read(button->device->receiver->fd, response_buf, SLAVERY_HIDPP_PACKET_LENGTH_LONG) !=
	    SLAVERY_HIDPP_PACKET_LENGTH_LONG) {
		return;
	}

	if (response_buf[2] == 0x8f) {
		return;
	}

	for (int i = 0; i < SLAVERY_HIDPP_PACKET_LENGTH_LONG; i++) {
		printf("%hhx ", response_buf[i]);
	}

	puts("");
}