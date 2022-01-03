/**
 * @file
 * @brief Unified receiver implementation.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#define _GNU_SOURCE

#include "receiver.h"

#include "button.h"
#include "device.h"
#include "event.h"
#include "feature.h"
#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/hidraw.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

const uint16_t SLAVERY_USB_VENDOR_ID_LOGITECH = 0x046d;
const uint16_t SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER = 0xc52b;

#if __GLIBC_MINOR__ < 32
	#include <ctype.h>
#endif

void slavery_receiver_array_free(slavery_receiver_t *receivers[], const ssize_t num_receivers) {
	log_debug("freeing receiver array at %p", receivers);

	for (int i = 0; i < num_receivers; i++) {
		slavery_receiver_free(receivers[i]);
	}

	free(receivers);
}

int slavery_receiver_free(slavery_receiver_t *receiver) {
	log_debug("freeing receiver %s", receiver->devnode);
	log_debug("stopping listener thread");

	if ((errno = pthread_cancel(receiver->listener_thread)) != 0) {
		if (errno == ESRCH) {
			log_debug("listener thread has already finished");
		} else {
			log_warning_errno(SLAVERY_ERROR_OS, "pthread_cancel()");

			return -1;
		}
	}

	if ((errno = pthread_join(receiver->listener_thread, NULL)) != 0) {
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

ssize_t slavery_receiver_scan_devices(slavery_receiver_t *receiver) {
	log_debug("getting devices connected to receiver %s...", receiver->devnode);

	receiver->num_devices = 0;
	receiver->devices = NULL;

	for (uint8_t device_index = SLAVERY_DEVICE_INDEX_1; device_index <= SLAVERY_DEVICE_INDEX_6;
	     device_index++) {
		slavery_device_t *device = slavery_receiver_get_device(receiver, device_index);

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

slavery_device_t *slavery_receiver_get_device(slavery_receiver_t *receiver, const uint8_t device_index) {
	log_debug("trying to communicate with device on %s:%u...", receiver->devnode, device_index);

	slavery_device_t *device = malloc(sizeof(slavery_device_t));
	device->receiver = receiver;
	device->index = device_index;

	if (slavery_device_get_features(device) < 0) {
		log_debug("failed to get device features for %s:%u", receiver->devnode, device_index);

		free(device);

		return NULL;
	}

	if (slavery_device_get_protocol_version(device) == NULL) {
		log_debug("failed to get device protocol for %s:%u", receiver->devnode, device_index);

		free(device);

		return NULL;
	}

	if (slavery_device_get_type(device) == SLAVERY_DEVICE_TYPE_UNKNOWN) {
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

	if (slavery_device_get_name(device) == NULL) {
		log_debug("failed to get device name for %s:%u", receiver->devnode, device_index);

		free(device->protocol_version);
		free(device);

		return NULL;
	}

	if (slavery_device_get_num_buttons(device) == 0) {
		log_debug("failed to get number of buttons for %s:%u", receiver->devnode, device_index);

		free(device->protocol_version);
		free(device->name);
		free(device);

		return NULL;
	}

	log_debug("detecting buttons for %s:%u...", receiver->devnode, device_index);

	device->buttons = malloc(device->num_buttons * sizeof(slavery_button_t *));

	for (size_t i = 0; i < device->num_buttons; i++) {
		device->buttons[i] = slavery_device_get_button(device, i);

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
			slavery_device_remap_button(device, device->buttons[i]);
		}
	}

	return device;
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

	if (slavery_receiver_get_report_descriptor(receiver) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to get report descriptor");

		close(receiver->control_pipe[0]);
		close(receiver->control_pipe[1]);
		close(receiver->fd);
		free(receiver->name);
		free(receiver->address);
		free(receiver);

		return NULL;
	}

	log_debug("found receiver %s on %s", receiver->name, receiver->devnode);
	log_debug("starting receiver listener thread...");

	if ((errno = pthread_create(
	         &receiver->listener_thread, NULL, (pthread_callback_t)slavery_receiver_listen, receiver)) != 0) {
		log_warning_errno(SLAVERY_ERROR_OS, "pthread_create() failed");

		close(receiver->control_pipe[0]);
		close(receiver->control_pipe[1]);
		close(receiver->fd);
		free(receiver->name);
		free(receiver->address);
		free(receiver);

		return NULL;
	}

	log_debug("receiver listener thread started");

	// virtual_input_create_device();

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

// FIXME: hidraw read read/writes full records, my pipe doesn't necessarily
// FIXME: sending control events to pipe when not actively requested -> pipe grows and isn't read from.
void *slavery_receiver_listen(slavery_receiver_t *receiver) {
	if ((errno = pthread_setname_np(pthread_self(), "listener")) != 0) {
		log_warning_errno(SLAVERY_ERROR_OS, "pthread_setname_np() failed");
	}

	log_debug("started");

	uint8_t response_data[SLAVERY_PACKET_LENGTH_MAX];

	while (true) {
		ssize_t response_size = read(receiver->fd, response_data, SLAVERY_PACKET_LENGTH_MAX);

		if (response_size < 0) {
			log_warning_errno(SLAVERY_ERROR_IO, "read()");

			return NULL;
		}

		switch (response_data[0]) {
			case SLAVERY_REPORT_ID_EVENT: {
#ifdef DEBUG
				char hex[response_size * 5];

				log_debug("starting worker to handle event of size %ld: %s",
				          response_size,
				          bytes_to_hex(response_data, response_size, hex));
#endif

				pthread_attr_t attr;

				if ((errno = pthread_attr_init(&attr)) != 0) {
					log_warning_errno(SLAVERY_ERROR_OS, "pthread_attr_init() failed");

					return NULL;
				}

				if ((errno = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
					log_warning_errno(SLAVERY_ERROR_OS, "pthread_attr_setdetachstate() failed");

					return NULL;
				}

				slavery_event_t *event = malloc(sizeof(slavery_event_t));
				event->receiver = receiver;
				event->size = response_size;
				event->data = malloc(response_size);
				memcpy(event->data, response_data, response_size);

				if ((errno = pthread_create(
				         (pthread_t[]){0}, &attr, (pthread_callback_t)slavery_event_dispatch, event)) != 0) {
					log_warning_errno(SLAVERY_ERROR_OS, "pthread_create() failed");

					free(event->data);
					free(event);

					return NULL;
				}

				if ((errno = pthread_attr_destroy(&attr)) != 0) {
					log_warning_errno(SLAVERY_ERROR_OS, "pthread_attr_destroy() failed");

					free(event->data);
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

	if (response_data[2] == SLAVERY_FEATURE_INDEX_ERROR) {
		if (response_data[5] == SLAVERY_HIDPP_ERROR_RESOURCE) {
			log_debug("received resource error, device likely doesn't exist");
		} else {
			log_warning(SLAVERY_ERROR_IO, "received error code");
		}

		return -1;
	}

#ifdef DEBUG
	char hex[SLAVERY_PACKET_LENGTH_CONTROL_LONG * 5];

	log_debug("received control event of size %u: %s",
	          SLAVERY_PACKET_LENGTH_CONTROL_LONG,
	          bytes_to_hex(response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG, hex));
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
