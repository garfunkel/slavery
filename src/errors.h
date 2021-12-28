#pragma once

#define ERROR_MAP(ERROR)                               \
	ERROR(SLAVERY_ERROR_EVENT, "Event error")          \
	ERROR(SLAVERY_ERROR_CONFIG, "Configuration error") \
	ERROR(SLAVERY_ERROR_IO, "IO error")                \
	ERROR(SLAVERY_ERROR_OS, "OS error")                \
	ERROR(SLAVERY_ERROR_HIDPP, "HID++ protocol error") \
	ERROR_UNKNOWN(SLAVERY_ERROR_UNKNOWN, "Unknown error")

typedef enum
{
#define ERROR(error_id, error_string) error_id,
#define ERROR_UNKNOWN(error_id, error_string) error_id
	ERROR_MAP(ERROR)
#undef ERROR
#undef ERROR_UNKNOWN
} slavery_error_t;

#pragma weak slavery_error_to_string
const char *slavery_error_to_string(const slavery_error_t error) {
	switch (error) {
#define ERROR(error_id, error_string) \
	case error_id:                    \
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
