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
