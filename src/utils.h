#pragma once

#include "errors.h"

#include <alloca.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Global macros.
 */
#define UNUSED(x) (void)(x)

/**
 * Utility functions.
 */
const char *bytes_to_hex(const uint8_t bytes[], const size_t num_bytes, char *restrict hex);

/**
 * Logging gubbins.
 */
extern const char *ERROR_LEVEL_DEBUG;
extern const char *ERROR_LEVEL_INFO;
extern const char *ERROR_LEVEL_WARNING;
extern const char *ERROR_LEVEL_ERROR;

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
	#define log_debug(...) log_x(ERROR_LEVEL_DEBUG, __FILE__, __func__, __LINE__, __VA_ARGS__)
	#define log_info(...) log_x(ERROR_LEVEL_INFO, __FILE__, __func__, __LINE, __VA_ARGS__)
#else
	#define log_debug(...)
	#define log_info(...)
#endif

#define log_warning(error, ...) \
	log_x_error(ERROR_LEVEL_WARNING, error, false, __FILE__, __func__, __LINE__, __VA_ARGS__)
#define log_warning_errno(error, ...) \
	log_x_error(ERROR_LEVEL_WARNING, error, true, __FILE__, __func__, __LINE__, __VA_ARGS__)

#define log_error(error, ...)                                                                \
	log_x_error(ERROR_LEVEL_ERROR, error, false, __FILE__, __func__, __LINE__, __VA_ARGS__); \
	exit(EXIT_FAILURE)
#define log_error_errno(error, ...)                                                         \
	log_x_error(ERROR_LEVEL_ERROR, error, true, __FILE__, __func__, __LINE__, __VA_ARGS__); \
	exit(EXIT_FAILURE)
