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
	slavery_t *slavery = slavery_new();
	ssize_t num_receivers = slavery_scan_receivers(slavery);

	for (ssize_t i = 0; i < num_receivers; i++) {
		slavery_receiver_t *receiver = slavery_get_receiver(slavery, i);
		ssize_t num_devices = slavery_receiver_scan_devices(receiver);

		for (ssize_t j = 0; j < num_devices; j++) {
			// slavery_device_set_config(device_entry->device, config);
		}

		while (getchar() != 'q') {
			continue;
		}
	}

	slavery_free(slavery);

	return EXIT_SUCCESS;
}
