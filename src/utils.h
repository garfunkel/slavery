#pragma once

#include <alloca.h>
#include <stddef.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0

#define UNUSED(x) (void)(x)

typedef uint8_t bool;

const char *bytes_to_hex(const uint8_t bytes[], const size_t num_bytes, char *hex);

#define ERROR_TABLE(ERROR)                     \
	ERROR(ERROR_EVENT, "Event error")          \
	ERROR(ERROR_CONFIG, "Configuration error") \
	ERROR(ERROR_IO, "IO error")                \
	ERROR(ERROR_OS, "OS error")                \
	ERROR(ERROR_UNKNOWN, "Unknown error")

typedef enum
{
#define ERROR_ID(error_id, error_string) error_id,
	ERROR_TABLE(ERROR_ID)
#undef ERROR_ID
} error_id_t;

const char *error_id_to_string(const error_id_t error_id);

void log_x(const char *level, const char *file, const char *func, const int line, const char *fmt, ...);
void log_x_error(const char *level,
                 const error_id_t error_id,
                 const bool with_errno,
                 const char *file,
                 const char *func,
                 const int line,
                 const char *fmt,
                 ...);

#ifdef DEBUG
	#define log_debug(...) log_x("DEBUG", __FILE__, __func__, __LINE__, __VA_ARGS__)
	#define log_info(...) log_x("INFO", __FILE__, __func__, __LINE, __VA_ARGS__)
#else
	#define log_debug(...)
	#define log_info(...)
#endif

#define log_warning(error_id, ...) \
	log_x_error("WARNING", error_id, FALSE, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define log_warning_errno(error_id, ...) \
	log_x_error("WARNING", error_id, TRUE, __FILE__, __func__, __LINE__, __VA_ARGS__)

#define log_error(error_id, ...)                                                      \
	log_x_error("ERROR", error_id, FALSE, __FILE__, __func__, __LINE__, __VA_ARGS__); \
	exit(EXIT_FAILURE)
#define log_error_errno(error_id, ...)                                               \
	log_x_error("ERROR", error_id, TRUE, __FILE__, __func__, __LINE__, __VA_ARGS__); \
	exit(EXIT_FAILURE)
