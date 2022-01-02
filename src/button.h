/**
 * @file
 * @brief Button functions and types.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct slavery_device_t slavery_device_t;

/**
 * @brief Describes button types.
 */
typedef enum
{
	SLAVERY_BUTTON_TYPE_BUTTON = 0x01,
	SLAVERY_BUTTON_TYPE_FUNCTION = 0x02,
	SLAVERY_BUTTON_TYPE_HOTKEY = 0x04,
	SLAVERY_BUTTON_TYPE_FUNCTION_TOGGLE = 0x08
} slavery_button_type_t;

#define CID_MAP(CID)                                        \
	CID(SLAVERY_CID_MOUSE_LEFT, 0x50, "left")               \
	CID(SLAVERY_CID_MOUSE_RIGHT, 0x51, "right")             \
	CID(SLAVERY_CID_MOUSE_MIDDLE, 0x52, "middle")           \
	CID(SLAVERY_CID_MOUSE_BACK, 0x53, "back")               \
	CID(SLAVERY_CID_MOUSE_FORWARD, 0x56, "forward")         \
	CID(SLAVERY_CID_MOUSE_THUMB, 0xc3, "thumb")             \
	CID(SLAVERY_CID_MOUSE_TOP, 0xc4, "top")                 \
	CID(SLAVERY_CID_MOUSE_SCROLL_UP, -2, "scroll_up")       \
	CID(SLAVERY_CID_MOUSE_SCROLL_DOWN, -3, "scroll_down")   \
	CID(SLAVERY_CID_MOUSE_SCROLL_LEFT, -4, "scroll_left")   \
	CID(SLAVERY_CID_MOUSE_SCROLL_RIGHT, -5, "scroll_right") \
	CID_UNKNOWN(SLAVERY_CID_MOUSE_UNKNOWN, -1, "unknown")

/**
 * @brief Describes command IDs.
 */
typedef enum
{
#define CID(cid_id, cid_value, cid_string) cid_id = cid_value,
#define CID_UNKNOWN(cid_id, cid_value, cid_string) cid_id
	CID_MAP(CID)
#undef CID
#undef CID_UNKNOWN
} slavery_cid_t;

#pragma weak slavery_cid_to_string
const char *slavery_cid_to_string(const slavery_cid_t cid) {
	switch (cid) {
#define CID(cid_id, cid_value, cid_string) \
	case cid_id:                           \
		return cid_string;
#define CID_UNKNOWN(cid_id, cid_value, cid_string) \
	default:                                       \
		return cid_string;
		CID_MAP(CID)
#undef CID
#undef CID_UNKNOWN
	}
}

#pragma weak slavery_string_to_cid
slavery_cid_t slavery_string_to_cid(const char *cid) {
#define CID(cid_id, cid_value, cid_string) \
	if (strcmp(cid, cid_string) == 0) {    \
		return cid_id;                     \
	}
#define CID_UNKNOWN(cid_id, cid_value, cid_string) return cid_id;
	CID_MAP(CID)
#undef CID
#undef CID_UNKNOWN
#undef CID_MAP
}

/**
 * @brief Describes a button.
 */
typedef struct slavery_button_t {
	slavery_device_t *device;
	uint8_t index;
	slavery_cid_t cid;
	uint16_t task_id;
	uint8_t flags;
	bool virtual;
	bool persistent_divert;
	bool temporary_divert;
	bool reprogrammable;
	slavery_button_type_t type;
	uint8_t function_position;
	uint8_t group;
	uint8_t group_remap_mask;
	bool gesture;
} slavery_button_t;
