#include "libslavery_p.h"
#include "monitor.h"
#include "receiver.h"
#include "utils.h"

#include <libudev.h>
#include <pthread.h>
#include <stdlib.h>

slavery_t *slavery_new() {
	slavery_t *slavery = malloc(sizeof(slavery_t));
	slavery->receivers = NULL;
	slavery->num_receivers = 0;
	slavery->monitor = slavery_monitor_new(slavery);

	return slavery;
}

int slavery_free(slavery_t *slavery) {
	slavery_receiver_array_free(slavery->receivers, slavery->num_receivers);

	free(slavery);

	return 0;
}

ssize_t slavery_scan_receivers(slavery_t *slavery) {
	log_debug("scanning for receivers...");

	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *device_list, *device_entry;

	slavery->num_receivers = 0;
	slavery->receivers = NULL;

	if ((udev = udev_new()) == NULL) {
		log_warning(SLAVERY_ERROR_UDEV, "udev_new() failed");

		return -1;
	}

	if ((enumerate = udev_enumerate_new(udev)) == NULL) {
		log_warning(SLAVERY_ERROR_UDEV, "udev_enumerate_new() failed");

		udev_unref(udev);

		return -1;
	}

	if (udev_enumerate_add_match_subsystem(enumerate, "hidraw") < 0) {
		log_warning(SLAVERY_ERROR_UDEV, "udev_enumerate_add_match_subsystem() failed");

		udev_enumerate_unref(enumerate);
		udev_unref(udev);

		return -1;
	}

	if (udev_enumerate_scan_devices(enumerate) < 0) {
		log_warning(SLAVERY_ERROR_UDEV, "udev_enumerate_scan_devices() failed");

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

		slavery->receivers =
		    realloc(slavery->receivers, sizeof(slavery_receiver_t) * (slavery->num_receivers + 1));
		slavery->receivers[slavery->num_receivers++] = receiver;

		udev_device_unref(device);
	}

	udev_enumerate_unref(enumerate);
	udev_unref(udev);

	log_debug("found %u receivers", slavery->num_receivers);

	return slavery->num_receivers;
}

slavery_receiver_t *slavery_get_receiver(slavery_t *slavery, size_t receiver_index) {
	return slavery->receivers[receiver_index];
}
