#pragma once

#include "globals.h"

#include <stdint.h>
#include <sys/types.h>

typedef enum
{
	SLAVERY_REPORT_ID_SHORT = 0x10,
	SLAVERY_REPORT_ID_LONG = 0x11
} slavery_report_id_t;

typedef struct slavery_receiver_t slavery_receiver_t;

typedef struct slavery_receiver_list_t {
	slavery_receiver_t *receiver;
	struct slavery_receiver_list_t *next;
} slavery_receiver_list_t;

typedef struct slavery_config_t slavery_config_t;

typedef struct slavery_device_t slavery_device_t;

typedef struct slavery_device_list_t {
	slavery_device_t *device;
	struct slavery_device_list_t *next;
} slavery_device_list_t;

typedef struct slavery_button_t slavery_button_t;

slavery_receiver_list_t *slavery_scan_receivers();
void slavery_receiver_list_free(slavery_receiver_list_t *receiver_list);
ssize_t slavery_receiver_list_length(const slavery_receiver_list_t *receiver_list);
void slavery_receiver_list_print(const slavery_receiver_list_t *receiver_list);

void slavery_receiver_free(slavery_receiver_t *receiver);
void slavery_receiver_print(const slavery_receiver_t *receiver);
slavery_device_list_t *slavery_receiver_get_devices(slavery_receiver_t *receiver);

slavery_config_t *slavery_config_read(const char *path);
void slavery_config_print(const slavery_config_t *config);

slavery_device_t *slavery_device_from_receiver(slavery_receiver_t *receiver, const uint8_t device_index);
int slavery_device_set_config(slavery_device_t *device, const slavery_config_t *config);
void slavery_device_print(const slavery_device_t *device);
void slavery_device_free(slavery_device_t *device);
