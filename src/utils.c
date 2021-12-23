#define _GNU_SOURCE

#include "utils.h"

#include <execinfo.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

const char *bytes_to_hex(const uint8_t bytes[], const size_t num_bytes, char *hex) {
	if (hex == NULL) {
		hex = malloc(num_bytes * 5);
	}

	for (size_t i = 0; i < num_bytes - 1; i++) {
		sprintf(hex + (i * 5), "0x%02hhx ", bytes[i]);
	}

	sprintf(hex + ((num_bytes - 1) * 5), "0x%02hhx", bytes[num_bytes - 1]);

	return hex;
}

#define ERROR_STRING(error_id, error_string) error_string,

const char *error_id_to_string(const error_id_t error_id) {
	static const char *error_table[] = {ERROR_TABLE(ERROR_STRING)};

#undef ERROR_TABLE
#undef ERROR_STRING

	if (error_id < 0 || error_id >= ERROR_UNKNOWN) {
		return error_table[ERROR_UNKNOWN];
	}

	return error_table[error_id];
}

void log_xf(const char *level, const char *file, const char *func, const int line, const char *fmt, ...) {
	char *msg = NULL;
	va_list args;

	va_start(args, fmt);
	vasprintf(&msg, fmt, args);
	va_end(args);

	fprintf(stderr, "%s: %s:%s:%d: %s\n", level, file, func, line, msg);

	free(msg);
}

void log_xfe(const char *level,
             const error_id_t error_id,
             const char *file,
             const char *func,
             const int line,
             const char *fmt,
             ...) {
	char *msg = NULL;
	va_list args;

	va_start(args, fmt);
	vasprintf(&msg, fmt, args);
	va_end(args);

	fprintf(stderr, "%s: %s:%s:%d: %s: %s\n", level, file, func, line, error_id_to_string(error_id), msg);

	free(msg);
}
