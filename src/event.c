/**
 * @file
 * @brief Event dispatch implementation.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#define _GNU_SOURCE

#include "event.h"

#include "button.h"
#include "device.h"
#include "receiver.h"
#include "utils.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *slavery_event_dispatch(slavery_event_t *event) {
	if (pthread_setname_np(pthread_self(), "event") != 0) {
		log_warning(SLAVERY_ERROR_OS, "pthread_setname_np() failed");
	}

	log_debug("started");

	slavery_device_t *device = NULL;

	for (size_t i = 0; i < event->receiver->num_devices; i++) {
		if (event->receiver->devices[i]->index == event->data[1]) {
			device = event->receiver->devices[i];
		}
	}

	if (device) {
		log_debug("received event for device %u", device->index);

		slavery_button_t *buttons[device->num_buttons];
		size_t num_pressed = 0;

		for (size_t i = 0; i < device->num_buttons; i++) {
			// Determine which buttons pressed.
			if (event->data[3] & 0x01 && device->buttons[i]->cid == SLAVERY_CID_MOUSE_LEFT) {
				buttons[num_pressed++] = device->buttons[i];
			} else if (event->data[3] & 0x02 && device->buttons[i]->cid == SLAVERY_CID_MOUSE_RIGHT) {
				buttons[num_pressed++] = device->buttons[i];
			} else if (event->data[3] & 0x04 && device->buttons[i]->cid == SLAVERY_CID_MOUSE_MIDDLE) {
				buttons[num_pressed++] = device->buttons[i];
			} else if (event->data[3] & 0x08 && device->buttons[i]->cid == SLAVERY_CID_MOUSE_BACK) {
				buttons[num_pressed++] = device->buttons[i];
			} else if (event->data[3] & 0x10 && device->buttons[i]->cid == SLAVERY_CID_MOUSE_FORWARD) {
				buttons[num_pressed++] = device->buttons[i];
			}
		}

		printf("buttons: %lu\n", num_pressed);

		// virtual_input(num_pressed > 0 ? 1 : 0);
	} else {
		log_debug("received event for an unrecognised device %u", event->data[1]);
	}

	free(event->data);
	free(event);

	return NULL;
}
