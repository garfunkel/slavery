#pragma once

#include "libslavery.h"
#include "libslavery_p.h"

#define ERROR_MAP(ERROR)                                     \
	ERROR(SLAVERY_HIDPP_ERROR_SUCCESS, 0x00, "")             \
	ERROR(SLAVERY_HIDPP_ERROR_INVALID_FEATURE, 0x01, "")     \
	ERROR(SLAVERY_HIDPP_ERROR_INVALID_FUNCTION, 0x02, "")    \
	ERROR(SLAVERY_HIDPP_ERROR_INVALID_VALUE, 0x03, "")       \
	ERROR(SLAVERY_HIDPP_ERROR_CONNECTION_FAILED, 0x04, "")   \
	ERROR(SLAVERY_HIDPP_ERROR_TOO_MANY_DEVICES, 0x05, "")    \
	ERROR(SLAVERY_HIDPP_ERROR_ALREADY_EXISTS, 0x06, "")      \
	ERROR(SLAVERY_HIDPP_ERROR_BUSY, 0x07, "")                \
	ERROR(SLAVERY_HIDPP_ERROR_UNKNOWN_DEVICE, 0x08, "")      \
	ERROR(SLAVERY_HIDPP_ERROR_RESOURCE, 0x09, "")            \
	ERROR(SLAVERY_HIDPP_ERROR_REQUEST_UNAVAILABLE, 0x10, "") \
	ERROR(SLAVERY_HIDPP_ERROR_INVALID_PARAM, 0x11, "")       \
	ERROR(SLAVERY_HIDPP_ERROR_WRONG_PIN, 0x12, "")           \
	ERROR(SLAVERY_HIDPP_ERROR_RESERVED, 0x13, "")

typedef enum
{
#define ERROR(error_id, error_value, error_string) error_id,
	ERROR_MAP(ERROR)
#undef ERROR
	ERROR_HIDPP_ERROR_UNKNOWN
} slavery_hidpp_error_t;

#pragma weak slavery_hidpp_error_to_string
const char *slavery_hidpp_error_to_string(const slavery_hidpp_error_t error) {
	switch (error) {
#define ERROR(error_id, error_value, error_string) \
	case error_id:                                 \
		return error_string;
		ERROR_MAP(ERROR)
#undef ERROR
#undef ERROR_MAP
		default:
			return "Unknown error";
	}
}
