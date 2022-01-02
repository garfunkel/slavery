/**
 * @file
 * @brief Event functions and types.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#pragma once

#include <stdint.h>
#include <sys/types.h>

typedef struct slavery_receiver_t slavery_receiver_t;

/**
 * @brief Describes an event for the protocol event system.
 */
typedef struct slavery_event_t {
	slavery_receiver_t *receiver;
	size_t size;
	uint8_t *data;
} slavery_event_t;

void *slavery_event_dispatch(slavery_event_t *event);
