/**
 * @file
 * @brief Unified receiver functions and types.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#pragma once

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct slavery_device_t slavery_device_t;

/**
 * @brief Describes a unifying receiver.
 */
typedef struct slavery_receiver_t {
	char *devnode;
	uint16_t vendor_id;
	uint16_t product_id;
	char *name;
	char *address;
	size_t num_devices;
	slavery_device_t **devices;
	pthread_t listener_thread;
	int fd;
	int control_pipe[2];
} slavery_receiver_t;

int slavery_scan_receivers(slavery_receiver_t **receivers[]);
void slavery_receiver_array_free(slavery_receiver_t *receivers[], const ssize_t num_receivers);
int slavery_receiver_free(slavery_receiver_t *receiver);
int slavery_receiver_scan_devices(slavery_receiver_t *receiver);
slavery_device_t *slavery_receiver_get_device(slavery_receiver_t *receiver, const uint8_t device_index);
slavery_receiver_t *slavery_receiver_from_devnode(const char *devnode);
int slavery_receiver_get_report_descriptor(slavery_receiver_t *receiver);
void *slavery_receiver_listen(slavery_receiver_t *receiver);
int slavery_receiver_control_read_response(slavery_receiver_t *receiver,
                                           uint8_t response_data[],
                                           ssize_t response_size);
int slavery_receiver_control_write_response(slavery_receiver_t *receiver,
                                            uint8_t response_data[],
                                            ssize_t response_size);
