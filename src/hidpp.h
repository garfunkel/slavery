#pragma once

#include "libslavery.h"
#include "libslavery_p.h"

#include <string.h>

#define ERROR_MAP(ERROR)                                                        \
	ERROR(SLAVERY_HIDPP_ERROR_SUCCESS, 0x00, "Success")                         \
	ERROR(SLAVERY_HIDPP_ERROR_INVALID_FEATURE, 0x01, "Invalid feature")         \
	ERROR(SLAVERY_HIDPP_ERROR_INVALID_FUNCTION, 0x02, "Invalid function")       \
	ERROR(SLAVERY_HIDPP_ERROR_INVALID_VALUE, 0x03, "Invalid value")             \
	ERROR(SLAVERY_HIDPP_ERROR_CONNECTION_FAILED, 0x04, "Connection failed")     \
	ERROR(SLAVERY_HIDPP_ERROR_TOO_MANY_DEVICES, 0x05, "Too many devices")       \
	ERROR(SLAVERY_HIDPP_ERROR_ALREADY_EXISTS, 0x06, "Already exists")           \
	ERROR(SLAVERY_HIDPP_ERROR_BUSY, 0x07, "Busy")                               \
	ERROR(SLAVERY_HIDPP_ERROR_UNKNOWN_DEVICE, 0x08, "Unknown device")           \
	ERROR(SLAVERY_HIDPP_ERROR_RESOURCE, 0x09, "Resource error")                 \
	ERROR(SLAVERY_HIDPP_ERROR_REQUEST_UNAVAILABLE, 0x10, "Request unavailable") \
	ERROR(SLAVERY_HIDPP_ERROR_INVALID_PARAM, 0x11, "Invalid parameter")         \
	ERROR(SLAVERY_HIDPP_ERROR_WRONG_PIN, 0x12, "Wrong PIN")                     \
	ERROR(SLAVERY_HIDPP_ERROR_RESERVED, 0x13, "Reserved")                       \
	ERROR_UNKNOWN(SLAVERY_HIDPP_ERROR_UNKNOWN, "Unknown error")

typedef enum
{
#define ERROR(error_id, error_value, error_string) error_id = error_value,
#define ERROR_UNKNOWN(error_id, error_string) error_id
	ERROR_MAP(ERROR)
#undef ERROR
#undef ERROR_UNKNOWN
} slavery_hidpp_error_t;

#pragma weak slavery_hidpp_error_to_string
const char *slavery_hidpp_error_to_string(const slavery_hidpp_error_t error) {
	switch (error) {
#define ERROR(error_id, error_value, error_string) \
	case error_id:                                 \
		return error_string;
#define ERROR_UNKNOWN(error_id, error_string) \
	default:                                  \
		return error_string;
		ERROR_MAP(ERROR)
#undef ERROR
#undef ERROR_UNKNOWN
#undef ERROR_MAP
	}
}

#define BUTTON_MAP(BUTTON)                                        \
	BUTTON(SLAVERY_HIDPP_BUTTON_LEFT, 0x50, "left")               \
	BUTTON(SLAVERY_HIDPP_BUTTON_RIGHT, 0x51, "right")             \
	BUTTON(SLAVERY_HIDPP_BUTTON_MIDDLE, 0x52, "middle")           \
	BUTTON(SLAVERY_HIDPP_BUTTON_BACK, 0x53, "back")               \
	BUTTON(SLAVERY_HIDPP_BUTTON_FORWARD, 0x56, "forward")         \
	BUTTON(SLAVERY_HIDPP_BUTTON_THUMB, 0xc3, "thumb")             \
	BUTTON(SLAVERY_HIDPP_BUTTON_TOP, 0xc4, "top")                 \
	BUTTON(SLAVERY_HIDPP_BUTTON_SCROLL_UP, -2, "scroll_up")       \
	BUTTON(SLAVERY_HIDPP_BUTTON_SCROLL_DOWN, -3, "scroll_down")   \
	BUTTON(SLAVERY_HIDPP_BUTTON_SCROLL_LEFT, -4, "scroll_left")   \
	BUTTON(SLAVERY_HIDPP_BUTTON_SCROLL_RIGHT, -5, "scroll_right") \
	BUTTON_UNKNOWN(SLAVERY_HIDPP_BUTTON_UNKNOWN, -1, "unknown")

typedef enum
{
#define BUTTON(button_id, button_value, button_string) button_id = button_value,
#define BUTTON_UNKNOWN(button_id, button_value, button_string) button_id
	BUTTON_MAP(BUTTON)
#undef BUTTON
#undef BUTTON_UNKNOWN
} slavery_hidpp_button_t;

#pragma weak slavery_hidpp_button_to_string
const char *slavery_hidpp_button_to_string(const slavery_hidpp_button_t button) {
	switch (button) {
#define BUTTON(button_id, button_value, button_string) \
	case button_id:                                    \
		return button_string;
#define BUTTON_UNKNOWN(button_id, button_value, button_string) \
	default:                                                   \
		return button_string;
		BUTTON_MAP(BUTTON)
#undef BUTTON
#undef BUTTON_UNKNOWN
	}
}

#pragma weak slavery_hidpp_string_to_button
slavery_hidpp_button_t slavery_hidpp_string_to_button(const char *button) {
#define BUTTON(button_id, button_value, button_string) \
	if (strcmp(button, button_string) == 0) {          \
		return button_id;                              \
	}
#define BUTTON_UNKNOWN(button_id, button_value, button_string) return button_id;
	BUTTON_MAP(BUTTON)
#undef BUTTON
#undef BUTTON_UNKNOWN
#undef BUTTON_MAP
}
