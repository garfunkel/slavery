#pragma once

#include <stdint.h>
#include <sys/types.h>

#define TRUE 1
#define FALSE 0

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

typedef struct slavery_device_t slavery_device_t;

typedef struct slavery_device_list_t {
	slavery_device_t *device;
	struct slavery_device_list_t *next;
} slavery_device_list_t;

slavery_receiver_list_t *slavery_scan_receivers();
void slavery_receiver_list_free(slavery_receiver_list_t *receiver_list);
ssize_t slavery_receiver_list_length(const slavery_receiver_list_t *receiver_list);
void slavery_receiver_list_print(const slavery_receiver_list_t *receiver_list);

void slavery_receiver_free(slavery_receiver_t *receiver);
void slavery_receiver_print(const slavery_receiver_t *receiver);
slavery_device_list_t *slavery_receiver_get_devices(slavery_receiver_t *receiver);

slavery_device_t *slavery_device_from_receiver(slavery_receiver_t *receiver, const uint8_t device_index);
void slavery_device_print(const slavery_device_t *device);
void slavery_device_free(slavery_device_t *device);
