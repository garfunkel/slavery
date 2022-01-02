/**
 * @file
 * @brief Implementation of a virtual input device, used to allow for button remapping.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <stdio.h>
#include <unistd.h>

struct libevdev *dev;
struct libevdev_uinput *uidev;

int virtual_input_create_device() {
	dev = libevdev_new();
	libevdev_set_name(dev, "fake keyboard device");

	if (libevdev_enable_event_type(dev, EV_KEY) != 0) {
		perror("error");

		return -1;
	}

	if (libevdev_enable_event_code(dev, EV_KEY, KEY_F12, NULL) != 0) {
		perror("error");

		return -1;
	}

	if (libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev) != 0) {
		perror("error");

		return -1;
	}

	return 0;
}

int virtual_input(const int on) {
	if (libevdev_uinput_write_event(uidev, EV_KEY, KEY_F12, on) < 0) {
		perror("error");

		return -1;
	}

	if (libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0) < 0) {
		perror("error");

		return -1;
	}

	/*if (libevdev_uinput_write_event(uidev, EV_KEY, KEY_F12, 0) < 0) {
	    perror("error");

	    return -1;
	}

	if (libevdev_uinput_write_event(uidev, EV_SYN, SYN_REPORT, 0) < 0) {
	    perror("error");

	    return -1;
	}*/

	// libevdev_uinput_destroy(uidev);

	printf("Complete\n");

	return 0;
}
