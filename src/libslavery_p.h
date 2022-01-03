#pragma once

#include <sys/types.h>

typedef struct slavery_receiver_t slavery_receiver_t;
typedef struct slavery_monitor_t slavery_monitor_t;

typedef struct slavery_t {
	size_t num_receivers;
	slavery_receiver_t **receivers;
	slavery_monitor_t *monitor;
} slavery_t;

slavery_t *slavery_new();
int slavery_free(slavery_t *slavery);
ssize_t slavery_scan_receivers(slavery_t *slavery);
slavery_receiver_t *slavery_get_receiver(slavery_t *slavery, size_t receiver_index);
