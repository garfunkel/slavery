/**
 * @brief Various utilities, macros, and constants.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Allow variables to be marked as unused to avoid compiler warnings.
 */
#define UNUSED(x) (void)(x)

/**
 * @brief Signature for pthread handler function.
 */
#define pthread_callback_t void *(*)(void *)

/**
 * @brief Set out information for HID++ protocol errors.
 */
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

/**
 * @brief Describes HID++ protocol errors returned by logitech.
 */
typedef enum
{
#define ERROR(error_id, error_value, error_string) error_id = error_value,
#define ERROR_UNKNOWN(error_id, error_string) error_id
	ERROR_MAP(ERROR)
#undef ERROR
#undef ERROR_UNKNOWN
} slavery_hidpp_error_t;

/**
 * @brief HID++ error to string.
 */
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

/**
 * @brief USB HID report IDs.
 */
typedef enum
{
	SLAVERY_REPORT_ID_CONTROL_SHORT = 0x10,
	SLAVERY_REPORT_ID_CONTROL_LONG = 0x11,
	SLAVERY_REPORT_ID_EVENT = 0x20
} slavery_report_id_t;

/**
 * @brief Converts byte array to human-readable hex string.
 *
 * @param bytes Byte array to convert.
 * @param num_bytes Size of byte array.
 * @param hex String to place output in, or NULL to allocate a new string.
 * @return const char* Human-readable hex string.
 */
const char *bytes_to_hex(const uint8_t bytes[], const size_t num_bytes, char *restrict hex);

/**
 * @brief Debug logging level.
 */
extern const char *LOG_LEVEL_DEBUG;

/**
 * @brief Info logging level.
 */
extern const char *LOG_LEVEL_INFO;

/**
 * @brief Warning logging level.
 */
extern const char *LOG_LEVEL_WARNING;

/**
 * @brief Error logging level. When used will exit the program with EXIT_FAILURE.
 */
extern const char *LOG_LEVEL_ERROR;

#define ERROR_MAP(ERROR)                               \
	ERROR(SLAVERY_ERROR_EVENT, "Event error")          \
	ERROR(SLAVERY_ERROR_CONFIG, "Configuration error") \
	ERROR(SLAVERY_ERROR_IO, "IO error")                \
	ERROR(SLAVERY_ERROR_OS, "OS error")                \
	ERROR(SLAVERY_ERROR_UDEV, "udev error")            \
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

void log_x(const char *level, const char *file, const char *func, const int line, const char *fmt, ...);
void log_x_error(const char *level,
                 const slavery_error_t error,
                 const bool with_errno,
                 const char *file,
                 const char *func,
                 const int line,
                 const char *fmt,
                 ...);

#ifdef DEBUG
	#define log_debug(...) log_x(LOG_LEVEL_DEBUG, __FILE__, __func__, __LINE__, __VA_ARGS__)
	#define log_info(...) log_x(LOG_LEVEL_INFO, __FILE__, __func__, __LINE, __VA_ARGS__)
#else
	#define log_debug(...)
	#define log_info(...)
#endif

#define log_warning(error, ...) \
	log_x_error(LOG_LEVEL_WARNING, error, false, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define log_warning_errno(error, ...) \
	log_x_error(LOG_LEVEL_WARNING, error, true, __FILE__, __func__, __LINE__, __VA_ARGS__)

#define log_error(error, ...)                                                              \
	log_x_error(LOG_LEVEL_ERROR, error, false, __FILE__, __func__, __LINE__, __VA_ARGS__); \
	exit(EXIT_FAILURE)
#define log_error_errno(error, ...)                                                       \
	log_x_error(LOG_LEVEL_ERROR, error, true, __FILE__, __func__, __LINE__, __VA_ARGS__); \
	exit(EXIT_FAILURE)

extern const uint16_t SLAVERY_USB_VENDOR_ID_LOGITECH;
extern const uint16_t SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER;

extern const size_t SLAVERY_PACKET_LENGTH_CONTROL_SHORT;
extern const size_t SLAVERY_PACKET_LENGTH_CONTROL_LONG;
extern const size_t SLAVERY_PACKET_LENGTH_EVENT;
extern const size_t SLAVERY_PACKET_LENGTH_MAX;

extern const uint8_t SLAVERY_SOFTWARE_ID;
