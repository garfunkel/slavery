#pragma once

#include "globals.h"

#include <sys/types.h>

typedef struct json_object json_object;

typedef struct slavery_config_action_t {
	char *action_name;
	bool pause_cursor;
} slavery_config_action_t;

typedef struct slavery_config_button_t {
	char *button_name;
	ssize_t num_actions;
	slavery_config_action_t **actions;
} slavery_config_button_t;

typedef struct slavery_config_t {
	ssize_t num_buttons;
	slavery_config_button_t **buttons;
} slavery_config_t;

extern const char *SLAVERY_CONFIG_OPTION_PAUSE_CURSOR;

slavery_config_action_t *slavery_config_action_parse(const char *action_name, json_object *obj);
slavery_config_button_t *slavery_config_button_parse(const char *button_name, json_object *obj);
