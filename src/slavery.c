/**
 * @file
 * @brief Reference implementation of libslavery.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#include "libslavery.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
	slavery_receiver_t **receivers;
	int num_receivers = slavery_scan_receivers(&receivers);

	for (int i = 0; i < num_receivers; i++) {
		int num_devices = slavery_receiver_scan_devices(receivers[i]);

		for (int i = 0; i < num_devices; i++) {
			// slavery_device_set_config(device_entry->device, config);
		}

		while (getchar() != 'q') {
			continue;
		}
	}

	slavery_receiver_array_free(receivers, num_receivers);

	return EXIT_SUCCESS;
}
