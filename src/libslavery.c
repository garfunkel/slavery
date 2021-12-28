#define _GNU_SOURCE

#include "libslavery.h"

#include "hidpp.h"
#include "libslavery_p.h"

#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <linux/hidraw.h>
#include <pthread.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define pthread_callback_t void *(*)(void *)

const uint16_t SLAVERY_USB_VENDOR_ID_LOGITECH = 0x046d;
const uint16_t SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER = 0xc52b;

const size_t SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT = 7;
const size_t SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG = 20;
const size_t SLAVERY_HIDPP_PACKET_LENGTH_EVENT = 15;
const size_t SLAVERY_HIDPP_PACKET_LENGTH_MAX = 32;

const uint8_t SLAVERY_HIDPP_SOFTWARE_ID = 0x01;

static void slavery_receiver_listener_signal_handler(int signum) {
	log_debug("process received signal SIG%s, stopping listener...", sigabbrev_np(signum));

	if (gettid() == getpid()) {
		log_debug("main thread received signal SIG%s, exiting...", sigabbrev_np(signum));

		raise(signum);
	}
}

void slavery_receiver_array_free(slavery_receiver_t *receivers[], const ssize_t num_receivers) {
	log_debug("freeing receiver array at %p", receivers);

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

int slavery_receiver_free(slavery_receiver_t *receiver) {
	log_debug("freeing receiver at %p", receiver);
	log_debug("sending SIGINT to listener thread and joining...");

	if (pthread_kill(receiver->listener_thread, SIGINT) != 0) {
		log_warning_errno(SLAVERY_ERROR_OS, "pthread_kill()");

		return -1;
	}

	if (pthread_join(receiver->listener_thread, NULL) != 0) {
		log_warning_errno(SLAVERY_ERROR_OS, "pthread_join()");

		return -1;
	}

	log_debug("closing file descriptors");

	if (close(receiver->control_pipe[0]) < 0) {
		log_warning_errno(SLAVERY_ERROR_IO, "close()");

		return -1;
	}

	if (close(receiver->control_pipe[1]) < 0) {
		log_warning_errno(SLAVERY_ERROR_IO, "close()");

		return -1;
	}

	if (close(receiver->fd) < 0) {
		log_warning_errno(SLAVERY_ERROR_IO, "close()");

		return -1;
	}

	slavery_device_array_free(receiver->devices, receiver->num_devices);

	free(receiver->devnode);
	free(receiver->name);
	free(receiver->address);
	free(receiver);

	return 0;
}

void slavery_receiver_print(const slavery_receiver_t *receiver) {
	printf("Devnode: %s\n", receiver->devnode);
	printf("Vendor ID: %04hx\n", receiver->vendor_id);
	printf("Product ID: %04hx\n", receiver->product_id);
}

int slavery_scan_receivers(slavery_receiver_t **receivers[]) {
	log_debug("scanning for receivers...");

	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *device_list, *device_entry;
	int num_receivers = 0;
	*receivers = NULL;

	if ((udev = udev_new()) == NULL) {
		log_warning(SLAVERY_ERROR_IO, "udev_new() failed");

		return -1;
	}

	if ((enumerate = udev_enumerate_new(udev)) == NULL) {
		log_warning(SLAVERY_ERROR_IO, "udev_enumerate_new() failed");

		udev_unref(udev);

		return -1;
	}

	if (udev_enumerate_add_match_subsystem(enumerate, "hidraw") < 0) {
		log_warning(SLAVERY_ERROR_IO, "udev_enumerate_add_match_subsystem() failed");

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		return -1;
	}

	if (udev_enumerate_scan_devices(enumerate) < 0) {
		log_warning(SLAVERY_ERROR_IO, "udev_enumerate_scan_devices() failed");

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		return -1;
	}

	device_list = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(device_entry, device_list) {
		const char *sys_path = udev_list_entry_get_name(device_entry);
		struct udev_device *device = udev_device_new_from_syspath(udev, sys_path);
		const char *devnode = udev_device_get_devnode(device);

		log_debug("found devnode on %s, checking if it is a receiver", devnode);

		slavery_receiver_t *receiver = slavery_receiver_from_devnode(devnode);

		if (receiver == NULL) {
			log_debug("failed to create receiver from devnode %s, ignoring devnode", devnode);

			udev_device_unref(device);

			continue;
		}

		if (slavery_receiver_get_report_descriptor(receiver) < 0) {
			log_warning(SLAVERY_ERROR_IO, "failed to get report descriptor");

			slavery_receiver_free(receiver);
			udev_device_unref(device);

			continue;
		}

		*receivers = realloc(*receivers, sizeof(slavery_receiver_t) * (num_receivers + 1));
		*receivers[num_receivers++] = receiver;

		udev_device_unref(device);
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);

	log_debug("found %u receivers", num_receivers);

	return num_receivers;
}

slavery_receiver_t *slavery_receiver_from_devnode(const char *devnode) {
	log_debug("trying to create a receiver from devnod %s...", devnode);

	slavery_receiver_t *receiver = malloc(sizeof(slavery_receiver_t));
	struct hidraw_devinfo info;
	int length;

	if ((receiver->fd = open(devnode, O_RDWR)) < 0) {
		log_warning_errno(SLAVERY_ERROR_IO, "open() failed");

		free(receiver);

		return NULL;
	}

	if ((ioctl(receiver->fd, HIDIOCGRAWINFO, &info)) < 0) {
		log_warning(SLAVERY_ERROR_IO, "ioctl(HIDIOCGRAWINFO) failed");

		close(receiver->fd);
		free(receiver);

		return NULL;
	}

	if ((uint16_t)info.vendor != SLAVERY_USB_VENDOR_ID_LOGITECH ||
	    (uint16_t)info.product != SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER) {
		log_debug("found mismatching vendor ID/product ID, ignoring devnode");

		close(receiver->fd);
		free(receiver);

		return NULL;
	}

	receiver->devnode = strdup(devnode);
	receiver->vendor_id = info.vendor;
	receiver->product_id = info.product;
	receiver->name = malloc(256);

	if ((length = ioctl(receiver->fd, HIDIOCGRAWNAME(256), receiver->name)) < 0) {
		log_warning(SLAVERY_ERROR_IO, "ioctl(HIDIOCGRAWNAME) failed");

		close(receiver->fd);
		free(receiver->name);
		free(receiver);

		return NULL;
	}

	receiver->name = realloc(receiver->name, length);
	receiver->address = malloc(256);

	if ((length = ioctl(receiver->fd, HIDIOCGRAWPHYS(256), receiver->address)) < 0) {
		log_warning(SLAVERY_ERROR_IO, "ioctl(HIDIOCGRAWPHYS) failed");

		close(receiver->fd);
		free(receiver->name);
		free(receiver->address);
		free(receiver);

		return NULL;
	}

	receiver->address = realloc(receiver->address, length);
	receiver->num_devices = 0;
	receiver->devices = NULL;

	if (pipe2(receiver->control_pipe, O_DIRECT) < 0) {
		log_warning(SLAVERY_ERROR_IO, "pipe2() failed");

		close(receiver->fd);
		free(receiver->name);
		free(receiver->address);
		free(receiver);

		return NULL;
	}

	log_debug("found receiver %s on %s", receiver->name, receiver->devnode);
	log_debug("starting receiver listener thread...");

	if (pthread_create(
	        &receiver->listener_thread, NULL, (pthread_callback_t)slavery_receiver_listen, receiver) != 0) {
		log_warning(SLAVERY_ERROR_OS, "pthread_create() failed");

		close(receiver->control_pipe[0]);
		close(receiver->control_pipe[1]);
		close(receiver->fd);
		free(receiver->name);
		free(receiver->address);
		free(receiver);

		return NULL;
	}

	if (pthread_setname_np(receiver->listener_thread, "listener") != 0) {
		log_warning(SLAVERY_ERROR_OS, "pthread_setname_np() failed");
	}

	log_debug("receiver listener thread started");

	return receiver;
}

int slavery_receiver_get_report_descriptor(slavery_receiver_t *receiver) {
	log_debug("getting report descriptor for receiver %s...", receiver->devnode);

	struct hidraw_report_descriptor desc;

	if ((ioctl(receiver->fd, HIDIOCGRDESCSIZE, &desc.size)) < 0) {
		log_warning(SLAVERY_ERROR_IO, "ioctl(HIDIOCGRDESCSIZE) failed");

		return -1;
	}

	if ((ioctl(receiver->fd, HIDIOCGRDESC, &desc)) < 0) {
		log_warning(SLAVERY_ERROR_IO, "ioctl(HIDIOCGRDESC) failed");

		return -1;
	}

#ifdef DEBUG
	char hex[desc.size * 5];

	log_debug(
	    "received report descriptor of size %u: %s", desc.size, bytes_to_hex(desc.value, desc.size, hex));
#endif

	return 0;
}

int slavery_receiver_get_devices(slavery_receiver_t *receiver) {
	log_debug("getting devices connected to receiver %s...", receiver->devnode);

	receiver->num_devices = 0;
	receiver->devices = NULL;

	for (uint8_t device_index = SLAVERY_HIDPP_DEVICE_INDEX_1; device_index <= SLAVERY_HIDPP_DEVICE_INDEX_6;
	     device_index++) {
		slavery_device_t *device = slavery_device_from_receiver(receiver, device_index);

		if (device == NULL) {
			log_debug("no device on %s:%u", receiver->devnode, device_index);

			continue;
		}

		receiver->devices =
		    realloc(receiver->devices, sizeof(slavery_device_t) * (receiver->num_devices + 1));
		receiver->devices[receiver->num_devices++] = device;
	}

	log_debug("found %u devices on receiver %s", receiver->num_devices, receiver->devnode);

	return receiver->num_devices;
}

// FIXME: hidraw read read/writes full records, my pipe doesn't necessarily
// FIXME: sending control events to pipe when not actively requested -> pipe grows and isn't read from.
void *slavery_receiver_listen(slavery_receiver_t *receiver) {
	struct sigaction sig_action = {.sa_handler = slavery_receiver_listener_signal_handler,
	                               .sa_flags = SA_RESETHAND};
	sigaction(SIGINT, &sig_action, NULL);

	log_debug("started");

	while (true) {
		uint8_t *response_data = malloc(SLAVERY_HIDPP_PACKET_LENGTH_MAX);
		ssize_t response_size = read(receiver->fd, response_data, SLAVERY_HIDPP_PACKET_LENGTH_MAX);

		if (response_size < 0) {
			if (errno == EINTR) {
				log_debug("read() interrupted, exiting...");
			}

			free(response_data);

			return NULL;
		}

		// event->size = event_size;
		// event->data = realloc(event->data, event->size);

		switch (response_data[0]) {
			case SLAVERY_REPORT_ID_EVENT: {
#ifdef DEBUG
				char hex[response_size * 5];

				log_debug("starting worker to handle event of size %ld: %s",
				          response_size,
				          bytes_to_hex(response_data, response_size, hex));
#endif

				pthread_attr_t attr;

				if (pthread_attr_init(&attr) != 0) {
					log_warning(SLAVERY_ERROR_OS, "pthread_attr_init() failed");

					free(response_data);

					return NULL;
				}

				if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
					log_warning(SLAVERY_ERROR_OS, "pthread_attr_setdetachstate() failed");

					free(response_data);

					return NULL;
				}

				slavery_event_t *event = malloc(sizeof(slavery_event_t));
				event->response_size = response_size;
				event->response_data = response_data;

				if (pthread_create(
				        (pthread_t[]){0}, &attr, (pthread_callback_t)slavery_event_dispatch, event) != 0) {
					log_warning(SLAVERY_ERROR_OS, "pthread_create() failed");

					free(response_data);
					free(event);

					return NULL;
				}

				if (pthread_attr_destroy(&attr) != 0) {
					log_warning(SLAVERY_ERROR_OS, "pthread_attr_destroy() failed");

					free(response_data);
					free(event);

					return NULL;
				}

				break;
			}

			default: {
#ifdef DEBUG
				char hex[response_size * 5];

				log_debug("redirecting control event of size %ld: %s",
				          response_size,
				          bytes_to_hex(response_data, response_size, hex));
#endif

				slavery_receiver_control_write_response(receiver, response_data, response_size);
			}
		}
	}

	return NULL;
}

// FIXME add flow control.
int slavery_receiver_control_read_response(slavery_receiver_t *receiver,
                                           uint8_t response_data[],
                                           ssize_t response_size) {
	if (read(receiver->control_pipe[0], response_data, response_size) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response");

		return -1;
	}

	if (response_data[2] == SLAVERY_HIDPP_FEATURE_INDEX_ERROR) {
		if (response_data[5] == SLAVERY_HIDPP_ERROR_RESOURCE) {
			log_debug("received resource error, device likely doesn't exist");
		} else {
			log_warning(SLAVERY_ERROR_IO, "received error code");
		}

		return -1;
	}

#ifdef DEBUG
	char hex[SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG * 5];

	log_debug("received control event of size %u: %s",
	          SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG,
	          bytes_to_hex(response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG, hex));
#endif

	return 0;
}

// FIXME add flow control.
int slavery_receiver_control_write_response(slavery_receiver_t *receiver,
                                            uint8_t response_data[],
                                            ssize_t response_size) {
	if (write(receiver->control_pipe[1], response_data, response_size) < 0) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to write control event response");

		return -1;
	}

	return 0;
}

void *slavery_event_dispatch(slavery_event_t *event) {
	// free(event->data);
	free(event);

	return NULL;
}

void slavery_device_array_free(slavery_device_t *devices[], const ssize_t num_devices) {
	log_debug("freeing device array at %p", devices);

	for (ssize_t i = 0; i < num_devices; i++) {
		slavery_device_free(devices[i]);
	}

	free(devices);
}

slavery_device_t *slavery_device_from_receiver(slavery_receiver_t *receiver, const uint8_t device_index) {
	log_debug("trying to communicate with device on %s:%u...", receiver->devnode, device_index);

	slavery_device_t *device = malloc(sizeof(slavery_device_t));
	device->receiver = receiver;
	device->index = device_index;

	if (slavery_hidpp_get_features(device) < 0) {
		log_debug("failed to get device features for %s:%u", receiver->devnode, device_index);

		free(device);

		return NULL;
	}

	if (slavery_hidpp_get_protocol_version(device) == NULL) {
		log_debug("failed to get device protocol for %s:%u", receiver->devnode, device_index);

		free(device);

		return NULL;
	}

	if (slavery_hidpp_get_type(device) == SLAVERY_DEVICE_TYPE_UNKNOWN) {
		log_debug("failed to get device type for %s:%u", receiver->devnode, device_index);

		free(device->protocol_version);
		free(device);

		return NULL;
	}

	if (device->type != SLAVERY_DEVICE_TYPE_MOUSE) {
		log_debug("device %s:%u is not a mouse, ignoring device", receiver->devnode, device_index);

		free(device->protocol_version);
		free(device);

		return NULL;
	}

	if (slavery_hidpp_get_name(device) == NULL) {
		log_debug("failed to get device name for %s:%u", receiver->devnode, device_index);

		free(device->protocol_version);
		free(device);

		return NULL;
	}

	if (slavery_hidpp_controls_get_num_buttons(device) == 0) {
		log_debug("failed to get number of buttons for %s:%u", receiver->devnode, device_index);

		free(device->protocol_version);
		free(device->name);
		free(device);

		return NULL;
	}

	log_debug("detecting buttons for %s:%u...", receiver->devnode, device_index);

	device->buttons = malloc(device->num_buttons * sizeof(slavery_button_t *));

	for (size_t i = 0; i < device->num_buttons; i++) {
		device->buttons[i] = slavery_hidpp_controls_get_button(device, i);

		if (device->buttons[i] == NULL) {
			log_debug("failed to get information for button %u", i);

			free(device->buttons);
			free(device->protocol_version);
			free(device->name);
			free(device);

			return NULL;
		}

		// TODO: remove this - this is for testing remapping.
		if (device->buttons[i]->cid == 0x00c3) {
			slavery_hidpp_controls_button_remap(device->buttons[i]);
		}
	}

	return device;
}

void slavery_device_free(slavery_device_t *device) {
	log_debug("freeing device %s:%u...", device->receiver->devnode, device->index);

	free(device->protocol_version);
	free(device->name);

	for (uint8_t i = 0; i < device->num_buttons; i++) {
		free(device->buttons[i]);
	}

	free(device->buttons);
	free(device);
}

int slavery_device_set_config(slavery_device_t *device, const slavery_config_t *config) {
	UNUSED(device);
	UNUSED(config);

	return 0;
}

void slavery_device_print(const slavery_device_t *device) {
	printf("Device %d:\n", device->index);
	printf("Name: %s\n", device->name);
	printf("Protocol Version: %s\n", device->protocol_version);
	puts("\n");
}

slavery_feature_t *slavery_hidpp_get_feature(slavery_device_t *device,
                                             slavery_hidpp_feature_id_t feature_id) {
	log_debug("getting feature information for feature %s", slavery_hidpp_feature_id_to_string(feature_id));

	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          SLAVERY_HIDPP_FEATURE_INDEX_ROOT,
	                          slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_ROOT_GET_FEATURE_INDEX),
	                          feature_id >> 8,
	                          feature_id & 0xff,
	                          0x00};
	uint8_t response_data[SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request feature");

		return NULL;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return NULL;
	}

	if (response_data[4] == 0x00 && response_data[5] == 0x00 && response_data[6] == 0x00) {
		log_debug("feature doesn't exist");

		return NULL;
	}

	slavery_feature_t *feature = malloc(sizeof(slavery_feature_t));
	feature->id = feature_id;
	feature->index = response_data[4];
	feature->flags = response_data[5];
	feature->version = response_data[6];

	log_debug("received information for feature %s", slavery_hidpp_feature_id_to_string(feature_id));

	return feature;
}

ssize_t slavery_hidpp_get_features(slavery_device_t *device) {
	log_debug("getting features for device %s:%u...", device->receiver->devnode, device->index);
	log_debug("checking if device exists");

	device->features = NULL;
	device->num_features = 0;
	slavery_feature_t *feature;

	// Get root feature, which we already know the details of, but lets do it for completeness.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_ROOT)) == NULL) {
		log_debug("couldn't get root feature for device %s:%u", device->receiver->devnode, device->index);

		return -1;
	}

	device->features = realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
	device->features[device->num_features++] = feature;

	// Get feature set feature.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_FEATURE_SET)) == NULL) {
		log_debug(
		    "couldn't get feature set feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get firmware feature.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_FIRMWARE)) == NULL) {
		log_debug("couldn't get firmware feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get name feature.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_NAME_TYPE)) == NULL) {
		log_debug("couldn't get name feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get reset feature.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_RESET)) == NULL) {
		log_debug("couldn't get reset feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get crypto feature.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_CRYPTO)) == NULL) {
		log_debug("couldn't get crypto feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get battery feature.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_BATTERY)) == NULL) {
		log_debug("couldn't get battery feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get host feature.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_HOST)) == NULL) {
		log_debug("couldn't get host feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get controls v4 feature.
	if ((feature = slavery_hidpp_get_feature(device, SLAVERY_HIDPP_FEATURE_ID_CONTROLS_V4)) == NULL) {
		log_debug("couldn't get controls v4 set feature for device %s:%u",
		          device->receiver->devnode,
		          device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	log_debug("found %d features for device %s:%u...",
	          device->num_features,
	          device->receiver->devnode,
	          device->index);

	return device->num_features;
}

ssize_t slavery_hidpp_feature_id_to_index(slavery_device_t *device, const uint16_t id) {
	for (uint8_t i = 0; i < device->num_features; i++) {
		if (device->features[i]->id == id) {
			return device->features[i]->index;
		}
	}

	return -1;
}

const char *slavery_hidpp_get_protocol_version(slavery_device_t *device) {
	log_debug("getting protocol version for device %s:%u...", device->receiver->devnode, device->index);

	device->protocol_version = NULL;
	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          SLAVERY_HIDPP_FEATURE_INDEX_ROOT,
	                          slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_ROOT_GET_PROTOCOL_VERSION),
	                          0x00,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request device protocol");

		return NULL;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return NULL;
	}

	asprintf(&device->protocol_version, "%d.%d", response_data[4], response_data[5]);

	log_debug("device protocol %s", device->protocol_version);

	return device->protocol_version;
}

slavery_device_type_t slavery_hidpp_get_type(slavery_device_t *device) {
	log_debug("getting type for device %s:%u...", device->receiver->devnode, device->index);

	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          slavery_hidpp_feature_id_to_index(device, SLAVERY_HIDPP_FEATURE_ID_NAME_TYPE),
	                          slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_NAME_TYPE_GET_TYPE),
	                          0x00,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request type");

		return -1;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return SLAVERY_DEVICE_TYPE_UNKNOWN;
	}

	device->type = response_data[4];

	log_debug("device type %d", device->type);

	return device->type;
}

const char *slavery_hidpp_get_name(slavery_device_t *device) {
	log_debug("getting name length for device %s:%u...", device->receiver->devnode, device->index);

	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          slavery_hidpp_feature_id_to_index(device, SLAVERY_HIDPP_FEATURE_ID_NAME_TYPE),
	                          slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_NAME_TYPE_GET_NAME_LENGTH),
	                          0x00,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request name length");

		return NULL;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return NULL;
	}

	log_debug("device name length: %u", response_data[4]);

	uint8_t name_length = response_data[4];
	char *name = malloc(name_length + 1);
	char *name_pos = name;
	request_data[3] = slavery_hidpp_encode_function(SLAVERY_HIDPP_FUNCTION_NAME_TYPE_GET_NAME);

	log_debug("getting name for device %s:%u...", device->receiver->devnode, device->index);

	do {
		if (write(device->receiver->fd, request_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) !=
		    (ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) {
			log_warning_errno(SLAVERY_ERROR_IO, "failed to request name");

			return NULL;
		}

		if (slavery_receiver_control_read_response(
		        device->receiver, response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) < 0) {
			log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

			return NULL;
		}

		ssize_t num_bytes = (name + name_length + 1) - name_pos;

		if (num_bytes > 16) {
			num_bytes = 16;
		}

		name_pos = stpncpy(name_pos, (const char *)response_data + 4, num_bytes);
		request_data[4] += num_bytes;
	} while (request_data[4] < name_length);

	device->name = name;

	log_debug("device name: %s", device->name);

	return device->name;
}

ssize_t slavery_hidpp_controls_get_num_buttons(slavery_device_t *device) {
	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          slavery_hidpp_feature_id_to_index(device, SLAVERY_HIDPP_FEATURE_ID_CONTROLS_V4),
	                          slavery_hidpp_encode_function(0),
	                          0x00,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request number of buttons");

		return -1;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return -1;
	}

	device->num_buttons = response_data[4];

	return device->num_buttons;
}

slavery_button_t *slavery_hidpp_controls_get_button(slavery_device_t *device, uint8_t button_index) {
	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          slavery_hidpp_feature_id_to_index(device, SLAVERY_HIDPP_FEATURE_ID_CONTROLS_V4),
	                          slavery_hidpp_encode_function(1),
	                          button_index,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG];
	slavery_button_t *button;

	if (write(device->receiver->fd, request_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT) {
		return NULL;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return NULL;
	}

	button = malloc(sizeof(slavery_button_t));

	printf("get button info: ");

	for (size_t i = 0; i < SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG; i++) {
		printf("%hhx ", response_data[i]);
	}

	puts("");

	button->index = button_index;
	button->device = device;
	button->cid = (uint16_t)response_data[4] << 8 | response_data[5];
	button->task_id = (uint16_t)response_data[6] << 8 | response_data[7];
	button->flags = response_data[8];
	button->virtual = response_data[8] & 0x80;
	button->persistent_divert = response_data[8] & 0x40;
	button->temporary_divert = response_data[8] & 0x20;
	button->reprogrammable = response_data[8] & 0x10;
	button->type = button->flags & 0x0f;
	button->function_position = response_data[9];
	button->group = response_data[10];
	button->group_remap_mask = response_data[11];
	button->gesture = response_data[12];

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
	UNUSED(button);

	/*uint8_t request_data[] = {
	    SLAVERY_REPORT_ID_CONTROL_LONG,
	    button->device->index,
	    slavery_hidpp_feature_id_to_index(button->device, SLAVERY_HIDPP_FEATURE_ID_CONTROLS_V4),
	    slavery_hidpp_encode_function(3),
	    button->cid >> 8,
	    button->cid & 0xff,
	    / *
	    2 mouse move
	    3 stop mouse move

	    a remap
	    3 stop map

	    mapped to back:
	    event size: 15: 20 01 02 08 00 00 00 00 00 00 36 00 00 00 00
	    event size: 15: 20 01 02 00 00 00 00 00 00 00 3e 00 00 00 00

	    no map:
	    event size: 20: 11 01 09 00 00 c3 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	    event size: 20: 11 01 09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

	    back:
	    event size: 15: 20 01 02 08 00 00 00 00 00 00 36 00 00 00 00
	    event size: 15: 20 01 02 00 00 00 00 00 00 00 3e 00 00 00 00
	    * /
	0x2a,
	0x00, // button->cid >> 8,
	0x53, // button->cid & 0xff,
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
uint8_t response_data[SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG];

printf("reprog: %d, pers: %d, temp: %d\n",
	   button->reprogrammable,
	   button->persistent_divert,
	   button->temporary_divert);
printf("set button remap: ");

for (size_t i = 0; i < SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG; i++) {
	printf("%hhx ", request_data[i]);
}

puts("");

if (write(button->device->receiver->fd, request_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) !=
	(ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) {
	return;
}

if (read(button->device->receiver->control_pipe[0], response_data, SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG)
!= (ssize_t)SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG) { return;
}

if (response_data[2] == 0xff) {
	printf("remap error\n");

	exit(0);

	return;
}

for (size_t i = 0; i < SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG; i++) {
	printf("%hhx ", response_data[i]);
}

puts("");
*/
}
