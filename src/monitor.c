/**
 * @file
 * @brief Implementation for a monitor watching for udev device changes.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#include "monitor.h"

#include "libslavery_p.h"
#include "receiver.h"
#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

slavery_monitor_t *slavery_monitor_new(slavery_t *slavery) {
	slavery_monitor_t *monitor = malloc(sizeof(slavery_monitor_t));
	struct udev *udev;
	int fd;

	monitor->slavery = slavery;

	if ((udev = udev_new()) == NULL) {
		log_warning(SLAVERY_ERROR_UDEV, "udev_new() failed");

		free(monitor);

		return NULL;
	}

	if ((monitor->udev_monitor = udev_monitor_new_from_netlink(udev, "udev")) == NULL) {
		log_warning(SLAVERY_ERROR_UDEV, "udev_monitor_new_from_netlink() failed");

		udev_unref(udev);
		free(monitor);

		return NULL;
	}

	if (udev_monitor_filter_add_match_subsystem_devtype(monitor->udev_monitor, "hidraw", NULL) < 0) {
		log_warning(SLAVERY_ERROR_UDEV, "udev_monitor_filter_add_match_subsystem_devtype(), failed");

		udev_monitor_unref(monitor->udev_monitor);
		udev_unref(udev);
		free(monitor);

		return NULL;
	}

	if (udev_monitor_enable_receiving(monitor->udev_monitor) < 0) {
		log_warning(SLAVERY_ERROR_UDEV, "udev_monitor_enable_receiving() failed");

		udev_monitor_unref(monitor->udev_monitor);
		udev_unref(udev);
		free(monitor);

		return NULL;
	}

	fd = udev_monitor_get_fd(monitor->udev_monitor);

	if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK) == -1) {
		log_warning_errno(SLAVERY_ERROR_IO, "fcntl() failed");

		udev_monitor_unref(monitor->udev_monitor);
		udev_unref(udev);
		free(monitor);

		return NULL;
	}

	if ((errno = pthread_create(
	         &monitor->monitor_thread, NULL, (pthread_callback_t)slavery_monitor_run, monitor)) != 0) {
		log_warning_errno(SLAVERY_ERROR_OS, "pthread_create() failed");

		udev_monitor_unref(monitor->udev_monitor);
		udev_unref(udev);
		free(monitor);

		return NULL;
	}

	return monitor;
}

int slavery_monitor_free(slavery_monitor_t *monitor) {
	log_debug("freeing monitor at %p", monitor);
	log_debug("stopping monitor thread");

	if ((errno = pthread_cancel(monitor->monitor_thread)) != 0) {
		if (errno == ESRCH) {
			log_debug("monitor thread has already finished");
		} else {
			log_warning_errno(SLAVERY_ERROR_OS, "pthread_cancel()");

			return -1;
		}
	}

	if ((errno = pthread_join(monitor->monitor_thread, NULL)) != 0) {
		log_warning_errno(SLAVERY_ERROR_OS, "pthread_join()");

		return -1;
	}

	return 0;
}

void *slavery_monitor_run(slavery_monitor_t *monitor) {
	while (true) {
		struct udev_device *device = udev_monitor_receive_device(monitor->udev_monitor);
		const char *devnode = udev_device_get_devnode(device);
		const char *action = udev_device_get_action(device);

		if (strcmp(action, "add") == 0) {
			slavery_receiver_t *receiver = slavery_receiver_from_devnode(devnode);

			if (receiver == NULL) {
				log_debug("failed to create receiver from devnode %s, ignoring devnode", devnode);
			} else {
				if (slavery_receiver_scan_devices(receiver) < 0) {
					log_debug("failed to scan devices on receiver %s", devnode);
				} else {
					monitor->slavery->receivers =
					    realloc(monitor->slavery->receivers,
					            sizeof(slavery_receiver_t) * (monitor->slavery->num_receivers + 1));
					monitor->slavery->receivers[monitor->slavery->num_receivers++] = receiver;
				}
			}
		} else if (strcmp(action, "remove") == 0) {
			for (size_t i = 0; i < monitor->slavery->num_receivers; i++) {
				slavery_receiver_t *receiver = slavery_get_receiver(monitor->slavery, i);

				if (strcmp(receiver->devnode, devnode) == 0) {
					slavery_receiver_free(receiver);

					for (size_t j = i + 1; j < monitor->slavery->num_receivers; j++) {
						monitor->slavery->receivers[j - 1] = monitor->slavery->receivers[j];
					}

					monitor->slavery->num_receivers--;

					break;
				}
			}
		}

		printf("device change %s %s\n", devnode, action);

		udev_device_unref(device);
	}
}
