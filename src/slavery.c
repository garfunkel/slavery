#include "libslavery.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
	slavery_receiver_t **receivers;
	int num_receivers = slavery_scan_receivers(&receivers);

	slavery_receiver_array_print((const slavery_receiver_t **)receivers, num_receivers);

	for (int i = 0; i < num_receivers; i++) {
		slavery_device_t **devices;
		int num_devices = slavery_receiver_get_devices(receivers[i], &devices);

		slavery_receiver_start_listener(receivers[i]);

		for (int i = 0; i < num_devices; i++) {
			// slavery_device_set_config(device_entry->device, config);
		}

		// getchar();

		sleep(3);

		slavery_receiver_stop_listener(receivers[i]);
		slavery_device_array_free(devices, num_devices);
	}

	slavery_receiver_array_free(receivers, num_receivers);

	return EXIT_SUCCESS;
}
