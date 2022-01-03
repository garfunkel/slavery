/**
 * @file
 * @brief Monitors udev device changes.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#pragma once

#include <pthread.h>

typedef struct slavery_t slavery_t;
struct udev_monitor;

typedef struct slavery_monitor_t {
	slavery_t *slavery;
	struct udev_monitor *udev_monitor;
	pthread_t monitor_thread;
} slavery_monitor_t;

slavery_monitor_t *slavery_monitor_new(slavery_t *slavery);
int slavery_monitor_free(slavery_monitor_t *monitor);
void *slavery_monitor_run(slavery_monitor_t *monitor);
