#pragma once

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#define ACTION_MAP(ACTION)                                     \
	ACTION(SLAVERY_CONFIG_ACTION_PRESSED, "pressed")           \
	ACTION(SLAVERY_CONFIG_ACTION_HOLD, "hold")                 \
	ACTION(SLAVERY_CONFIG_ACTION_DOUBLE_TAP, "double_tap")     \
	ACTION(SLAVERY_CONFIG_ACTION_TRIPLE_TAP, "triple_tap")     \
	ACTION(SLAVERY_CONFIG_ACTION_RELEASED, "released")         \
	ACTION(SLAVERY_CONFIG_ACTION_GESTURE_UP, "gesture_up")     \
	ACTION(SLAVERY_CONFIG_ACTION_GESTURE_DOWN, "gesture_down") \
	ACTION(SLAVERY_CONFIG_ACTION_GESTURE_LEFT, "gesture_left") \
	ACTION(SLAVERY_CONFIG_ACTION_GESTURE_RIGHT, "gesture_right")

typedef enum
{
#define ACTION(action_id, action_string) action_id,
	ACTION_MAP(ACTION)
#undef ACTION
	SLAVERY_CONFIG_ACTION_UNKNOWN
} slavery_config_action_t;

#pragma weak slavery_config_action_to_string
const char *slavery_config_action_to_string(const slavery_config_action_t action) {
	switch (action) {
#define ACTION(action_id, action_string) \
	case action_id:                      \
		return action_string;
		ACTION_MAP(ACTION)
#undef ACTION
		default:
			return "Unknown action";
	}
}

#pragma weak slavery_config_string_to_action
slavery_config_action_t slavery_config_string_to_action(const char *action) {
#define ACTION(action_id, action_string)      \
	if (strcmp(action, action_string) == 0) { \
		return action_id;                     \
	}
	ACTION_MAP(ACTION)
#undef ACTION
#undef ACTION_MAP
	return SLAVERY_CONFIG_ACTION_UNKNOWN;
}

typedef struct json_object json_object;
typedef struct array_list array_list;

typedef struct slavery_config_button_t {
	int x;
	/*char *button_name;
	ssize_t num_actions;
	slavery_config_action_t **actions;*/
} slavery_config_button_t;

typedef struct slavery_config_entry_t {
	char *name;
	char *description;
	size_t num_buttons;
	slavery_config_button_t *buttons;
	size_t num_actions;
	slavery_config_action_t *actions;
	bool enabled;
	bool inhibit_cursor;
	bool do_default;
	size_t num_do_commands;
	char **do_command;
	size_t num_do_keyboard;
	char **do_keyboard;
	size_t num_do_mouse;
	char **do_mouse;
} slavery_config_entry_t;

typedef struct slavery_config_t {
	size_t num_entries;
	slavery_config_entry_t **entries;
} slavery_config_t;

slavery_config_entry_t *slavery_config_entry_parse(const char *name, const json_object *obj);
