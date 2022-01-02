/**
 * @file
 * @brief Device function implementations.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#pragma once

#include "feature.h"

#include <stdint.h>
#include <sys/types.h>

typedef struct slavery_receiver_t slavery_receiver_t;
typedef struct slavery_config_t slavery_config_t;
typedef struct slavery_button_t slavery_button_t;
typedef struct slavery_feature_t slavery_feature_t;

/**
 * @brief Device indexes supported by HID++.
 */
typedef enum
{
	SLAVERY_DEVICE_INDEX_1 = 0x01,
	SLAVERY_DEVICE_INDEX_2 = 0x02,
	SLAVERY_DEVICE_INDEX_3 = 0x03,
	SLAVERY_DEVICE_INDEX_4 = 0x04,
	SLAVERY_DEVICE_INDEX_5 = 0x05,
	SLAVERY_DEVICE_INDEX_6 = 0x06,
	SLAVERY_DEVICE_INDEX_RECEIVER = 0xff
} slavery_device_index_t;

#define DEVICE_TYPE_MAP(DEVICE_TYPE)                                  \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_KEYBOARD, "keyboard")             \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_REMOTE_CONTROL, "remote_control") \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_NUMPAD, "numpad")                 \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_MOUSE, "mouse")                   \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_TOUCHPAD, "touchpad")             \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_TRACKBALL, "trackball")           \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_PRESENTER, "presenter")           \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_RECEIVER, "receiver")             \
	DEVICE_TYPE_UNKNOWN(SLAVERY_DEVICE_TYPE_UNKNOWN, "unknown")

/**
 * @brief Describes device types.
 */
typedef enum
{
#define DEVICE_TYPE(device_type_id, device_type_string) device_type_id,
#define DEVICE_TYPE_UNKNOWN(device_type_id, device_type_string) device_type_id
	DEVICE_TYPE_MAP(DEVICE_TYPE)
#undef DEVICE_TYPE
#undef DEVICE_TYPE_UNKNOWN
} slavery_device_type_t;

#pragma weak slavery_device_type_to_string
const char *slavery_device_type_to_string(const slavery_device_type_t device_type) {
	switch (device_type) {
#define DEVICE_TYPE(device_type_id, device_type_string) \
	case device_type_id:                                \
		return device_type_string;
#define DEVICE_TYPE_UNKNOWN(device_type_id, device_type_string) \
	default:                                                    \
		return device_type_string;
		DEVICE_TYPE_MAP(DEVICE_TYPE)
#undef DEVICE_TYPE
#undef DEVICE_TYPE_UNKNOWN
#undef DEVICE_TYPE_MAP
	}
}

/**
 * @brief Describes a compatible device.
 */
typedef struct slavery_device_t {
	slavery_receiver_t *receiver;
	uint8_t index;
	char *protocol_version;
	slavery_device_type_t type;
	char *name;
	size_t num_features;
	slavery_feature_t **features;
	size_t num_buttons;
	slavery_button_t **buttons;
} slavery_device_t;

void slavery_device_array_free(slavery_device_t *devices[], const ssize_t num_devices);
int slavery_device_set_config(slavery_device_t *device, const slavery_config_t *config);
void slavery_device_free(slavery_device_t *device);
ssize_t slavery_device_get_features(slavery_device_t *device);
slavery_feature_t *slavery_device_get_feature(slavery_device_t *device, slavery_feature_id_t feature_id);
const char *slavery_device_get_protocol_version(slavery_device_t *device);
slavery_device_type_t slavery_device_get_type(slavery_device_t *device);
const char *slavery_device_get_name(slavery_device_t *device);
ssize_t slavery_device_get_num_buttons(slavery_device_t *device);
slavery_button_t *slavery_device_get_button(slavery_device_t *device, uint8_t button_index);
void slavery_device_remap_button(slavery_device_t *device, slavery_button_t *button);
ssize_t slavery_feature_id_to_index(slavery_device_t *device, const uint16_t id);
