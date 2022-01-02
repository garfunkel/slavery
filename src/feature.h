/**
 * @file
 * @brief Declaration of protocol features (function groups).
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#pragma once

#include <stdint.h>

#define FEATURE_ID_MAP(FEATURE_ID)                                    \
	FEATURE_ID(SLAVERY_FEATURE_ID_ROOT, 0x0000, "root")               \
	FEATURE_ID(SLAVERY_FEATURE_ID_FEATURE_SET, 0x0001, "feature_set") \
	FEATURE_ID(SLAVERY_FEATURE_ID_FIRMWARE, 0x0003, "firmware")       \
	FEATURE_ID(SLAVERY_FEATURE_ID_NAME_TYPE, 0x0005, "name/type")     \
	FEATURE_ID(SLAVERY_FEATURE_ID_RESET, 0x0020, "reset")             \
	FEATURE_ID(SLAVERY_FEATURE_ID_CRYPTO, 0x0021, "crypto")           \
	FEATURE_ID(SLAVERY_FEATURE_ID_BATTERY, 0x1000, "battery")         \
	FEATURE_ID(SLAVERY_FEATURE_ID_HOST, 0x1814, "host")               \
	FEATURE_ID(SLAVERY_FEATURE_ID_CONTROLS_V4, 0x1b04, "controls_v4") \
	FEATURE_ID_UNKNOWN(SLAVERY_FEATURE_ID_UNKNOWN, "unknown")

/**
 * @brief Supported features IDs.
 */
typedef enum
{
#define FEATURE_ID(feature_id_id, feature_id_value, feature_id_string) feature_id_id = feature_id_value,
#define FEATURE_ID_UNKNOWN(feature_id_id, feature_id_string) feature_id_id
	FEATURE_ID_MAP(FEATURE_ID)
#undef FEATURE_ID
#undef FEATURE_ID_UNKNOWN
} slavery_feature_id_t;

#pragma weak slavery_feature_id_to_string
const char *slavery_feature_id_to_string(const slavery_feature_id_t feature_id) {
	switch (feature_id) {
#define FEATURE_ID(feature_id_id, feature_id_value, feature_id_string) \
	case feature_id_id:                                                \
		return feature_id_string;
#define FEATURE_ID_UNKNOWN(feature_id_id, feature_id_string) \
	default:                                                 \
		return feature_id_string;
		FEATURE_ID_MAP(FEATURE_ID)
#undef FEATURE_ID
#undef FEATURE_ID_UNKNOWN
#undef FEATURE_ID_MAP
	}
}

/**
 * @brief HID++ static feature indexes.
 */
typedef enum
{
	SLAVERY_FEATURE_INDEX_ROOT = 0x00,
	SLAVERY_FEATURE_INDEX_ERROR = 0x8f
} slavery_feature_index_t;

/**
 * @brief Describes a feature.
 */
typedef struct slavery_feature_t {
	uint16_t id;
	uint8_t index;
	uint8_t version;
	uint8_t flags;
} slavery_feature_t;
