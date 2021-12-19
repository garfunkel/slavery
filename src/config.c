#define _POSIX_C_SOURCE 200809L

#include "config.h"

#include "libslavery.h"

#include <fcntl.h>
#include <json.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

const char *SLAVERY_CONFIG_OPTION_PAUSE_CURSOR = "pause_cursor";

static ssize_t json_object_num_keys(json_object *obj) {
	ssize_t num_keys = 0;

	json_object_object_foreach(obj, key, value) {
		UNUSED(key);
		UNUSED(value);

		num_keys++;
	}

	return num_keys;
}

slavery_config_action_t *slavery_config_action_parse(const char *action_name, json_object *obj) {
	slavery_config_action_t *config_action = malloc(sizeof(slavery_config_action_t));
	config_action->action_name = strdup(action_name);
	json_object *value;

	if (json_object_object_get_ex(obj, SLAVERY_CONFIG_OPTION_PAUSE_CURSOR, &value)) {
		config_action->pause_cursor = json_object_get_boolean(value);
	}

	return config_action;
}

slavery_config_button_t *slavery_config_button_parse(const char *button_name, json_object *obj) {
	slavery_config_button_t *config_button = malloc(sizeof(slavery_config_button_t));
	config_button->button_name = strdup(button_name);
	config_button->num_actions = json_object_num_keys(obj);
	config_button->actions = malloc(config_button->num_actions * sizeof(slavery_config_action_t *));

	ssize_t action_index = 0;

	json_object_object_foreach(obj, key, value) {
		config_button->actions[action_index++] = slavery_config_action_parse(key, value);
	}

	return config_button;
}

slavery_config_t *slavery_config_read(const char *path) {
	int fd = open(path, O_RDONLY);
	char *json;
	struct stat stat;
	json_object *obj;
	slavery_config_t *config;

	if (fd < 0) {
		return NULL;
	}

	if (fstat(fd, &stat) < 0) {
		return NULL;
	}

	if ((json = mmap(NULL, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == NULL) {
		return NULL;
	}

	if ((obj = json_tokener_parse(json)) == NULL) {
		return NULL;
	}

	config = malloc(sizeof(slavery_config_t));
	config->num_buttons = json_object_num_keys(obj);
	config->buttons = malloc(config->num_buttons * sizeof(slavery_config_button_t *));

	ssize_t button_index = 0;

	json_object_object_foreach(obj, key, value) {
		config->buttons[button_index++] = slavery_config_button_parse(key, value);
	}

	return config;
}

void slavery_config_print(const slavery_config_t *config) {
	printf("num buttons: %ld\n", config->num_buttons);

	for (ssize_t button_index = 0; button_index < config->num_buttons; button_index++) {
		for (ssize_t action_index = 0; action_index < config->buttons[button_index]->num_actions;
		     action_index++) {
		}
	}
}