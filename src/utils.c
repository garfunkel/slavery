/**
 * @file
 * @brief Implementation of various utility functions and constants.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#define _GNU_SOURCE

#include "utils.h"

#include <errno.h>
#include <execinfo.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t SLAVERY_PACKET_LENGTH_CONTROL_SHORT = 7;
const size_t SLAVERY_PACKET_LENGTH_CONTROL_LONG = 20;
const size_t SLAVERY_PACKET_LENGTH_EVENT = 15;
const size_t SLAVERY_PACKET_LENGTH_MAX = 32;

const uint8_t SLAVERY_SOFTWARE_ID = 0x01;

const char *LOG_LEVEL_DEBUG = "DEBUG";
const char *LOG_LEVEL_INFO = "INFO";
const char *LOG_LEVEL_WARNING = "WARNING";
const char *LOG_LEVEL_ERROR = "ERROR";

const char *bytes_to_hex(const uint8_t bytes[], const size_t num_bytes, char *restrict hex) {
	if (hex == NULL) {
		hex = malloc(num_bytes * 5);
	}

	for (size_t i = 0; i < num_bytes - 1; i++) {
		sprintf(hex + (i * 5), "0x%02hhx ", bytes[i]);
	}

	sprintf(hex + ((num_bytes - 1) * 5), "0x%02hhx", bytes[num_bytes - 1]);

	return hex;
}

void log_x(const char *level, const char *file, const char *func, const int line, const char *fmt, ...) {
	char *msg = NULL;
	char thread_name[16];
	va_list args;

	va_start(args, fmt);
	vasprintf(&msg, fmt, args);
	va_end(args);

	pthread_getname_np(pthread_self(), thread_name, 16);

	fprintf(stderr, "%s: %s:%s:%d: [thread: %s]: %s\n", level, file, func, line, thread_name, msg);

	free(msg);
}

void log_x_error(const char *level,
                 const slavery_error_t error,
                 const bool with_errno,
                 const char *file,
                 const char *func,
                 const int line,
                 const char *fmt,
                 ...) {
	int local_errno = errno;
	char *msg;
	char thread_name[16];
	va_list args;

	va_start(args, fmt);
	vasprintf(&msg, fmt, args);
	va_end(args);

	if (with_errno) {
		char *_msg;

		asprintf(&_msg, "%s: syscall(): \"%s\"", msg, strerror(local_errno));
		free(msg);

		msg = _msg;
	}

	pthread_getname_np(pthread_self(), thread_name, 16);

	fprintf(stderr,
	        "%s: %s:%s:%d: %s: [thread: %s]: %s\n",
	        level,
	        file,
	        func,
	        line,
	        slavery_error_to_string(error),
	        thread_name,
	        msg);

	free(msg);
}
