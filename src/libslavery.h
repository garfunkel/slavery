#pragma once

#include "globals.h"

#include <stdint.h>
#include <sys/types.h>

typedef struct slavery_receiver_t slavery_receiver_t;
typedef struct slavery_config_t slavery_config_t;
typedef struct slavery_device_t slavery_device_t;
typedef struct slavery_button_t slavery_button_t;
typedef struct slavery_listener_t slavery_listener_t;

int slavery_scan_receivers(slavery_receiver_t **receivers[]);

void slavery_receiver_array_free(slavery_receiver_t *receivers[], const ssize_t num_receivers);
void slavery_receiver_array_print(const slavery_receiver_t *receivers[], const ssize_t num_receivers);
int slavery_receiver_free(slavery_receiver_t *receiver);
void slavery_receiver_print(const slavery_receiver_t *receiver);
int slavery_receiver_get_devices(slavery_receiver_t *receiver, slavery_device_t **devices[]);

slavery_config_t *slavery_config_read(const char *path);
void slavery_config_print(const slavery_config_t *config);

slavery_device_t *slavery_device_from_receiver(slavery_receiver_t *receiver, const uint8_t device_index);
void slavery_device_array_free(slavery_device_t *devices[], const ssize_t num_devices);
int slavery_device_set_config(slavery_device_t *device, const slavery_config_t *config);
void slavery_device_print(const slavery_device_t *device);
void slavery_device_free(slavery_device_t *device);
