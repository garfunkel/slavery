#define _POSIX_C_SOURCE 200809L

#include "config.h"

#include "libslavery.h"

#include <fcntl.h>
#include <json.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>

/*static char *parse_string(const json_object *obj, const char *key, const bool required) {
    json_object *child_obj;
    char *value;

    if (!json_object_object_get_ex(obj, key, &child_obj)) {
        if (required) {
            log_warning(SLAVERY_ERROR_CONFIG, "config entry is missing required key %s", key);
        }

        return NULL;
    }

    if ((value = strdup(json_object_get_string(child_obj))) == NULL) {
        log_warning(SLAVERY_ERROR_CONFIG, "config entry key %s must have a string value", key);

        return NULL;
    }

    return value;
}

static bool parse_bool(const json_object *obj, const char *key, const bool required, const bool default_) {
    json_object *child_obj;

    if (!json_object_object_get_ex(obj, key, &child_obj)) {
        if (required) {
            log_warning(SLAVERY_ERROR_CONFIG, "config entry is missing required key %s", key);
        }

        return default_;
    }

    return json_object_get_boolean(child_obj);
}

static int parse_string_or_array(const json_object *obj, char **strings[]) {
    switch (json_object_get_type(obj)) {
        case json_type_string: {
            *strings = malloc(sizeof(char **));

            if ((*strings[0] = strdup(json_object_get_string(obj))) == NULL) {
                log_warning(SLAVERY_ERROR_CONFIG, "bla");

                return NULL;
            }

            break;
        }

        case json_type_array:

            break;

        default:
            log_warning("config entry has invalid type for key %s", key);
    }

    return -1;
}

static slavery_config_action_t string_to_action(const char *string) {
    if (strcmp(string, "pressed") == 0) {
        return SLAVERY_CONFIG_ACTION_PRESSED;
    } else if (strcmp(string, "hold") == 0) {
        return SLAVERY_CONFIG_ACTION_HOLD;
    } else if (strcmp(string, "double_tap") == 0) {
        return SLAVERY_CONFIG_ACTION_DOUBLE_TAP;
    } else if (strcmp(string, "triple_tap") == 0) {
        return SLAVERY_CONFIG_ACTION_TRIPLE_TAP;
    } else if (strcmp(string, "released") == 0) {
        return SLAVERY_CONFIG_ACTION_RELEASED;
    } else if (strcmp(string, "gesture_up") == 0) {
        return SLAVERY_CONFIG_ACTION_GESTURE_UP;
    } else if (strcmp(string, "gesture_down") == 0) {
        return SLAVERY_CONFIG_ACTION_GESTURE_DOWN;
    } else if (strcmp(string, "gesture_left") == 0) {
        return SLAVERY_CONFIG_ACTION_GESTURE_LEFT;
    } else if (strcmp(string, "gesture_right") == 0) {
        return SLAVERY_CONFIG_ACTION_GESTURE_RIGHT;
    } else {
        return SLAVERY_CONFIG_ACTION_UNKNOWN;
    }
}

static ssize_t parse_entry_action(const json_object *obj, slavery_config_action_t **actions) {
    json_object *child_obj;
    const char *str_value;
    ssize_t num_strings;
    char *strings[];

    if (!json_object_object_get_ex(obj, "action", &child_obj)) {
        log_warning(SLAVERY_ERROR_CONFIG, "config entry is missing required key action");

        return SLAVERY_CONFIG_ACTION_UNKNOWN;
    }

    if ((num_strings = parse_entry_string_or_array(child_obj, &strings)) < 0) {
        log_warning(SLAVERY_ERROR_CONFIG, "config entry key action has invalid value");

        return -1;
    }

    *actions = malloc(sizeof(slavery_config_action_t *) * num_strings);

    for (ssize_t i = 0; i < num_strings; i++) {
        *actions[i] = string_to_action(strings[i]);
    }

    // TODO free strings

    return num_strings;
}*/

static ssize_t slavery_config_entry_parse_string(const json_object *obj,
                                                 const char *key,
                                                 const bool required,
                                                 char **value) {
	json_object *child_obj;
	*value = NULL;

	if (!json_object_object_get_ex(obj, key, &child_obj)) {
		if (!required) {
			log_debug("config entry is missing optional string value for %s", key);

			return 0;
		}

		log_warning(SLAVERY_ERROR_CONFIG, "config entry is missing required string value for %s", key);

		return -1;
	}

	if (json_object_get_type(child_obj) != json_type_string) {
		log_warning(SLAVERY_ERROR_CONFIG, "config entry has invalid type for %s", key);

		return -1;
	}

	*value = strdup(json_object_get_string(child_obj));

	log_debug("config entry has valid string value for %s", key);

	return 1;
}

static ssize_t slavery_config_entry_parse_bool(const json_object *obj,
                                               const char *key,
                                               const bool required,
                                               bool *value) {
	json_object *child_obj;
	*value = NULL;

	if (!json_object_object_get_ex(obj, key, &child_obj)) {
		if (!required) {
			log_debug("config entry is missing optional bool value for %s", key);

			return 0;
		}

		log_warning(SLAVERY_ERROR_CONFIG, "config entry is missing required bool value for %s", key);

		return -1;
	}

	if (json_object_get_type(child_obj) != json_type_boolean) {
		log_warning(SLAVERY_ERROR_CONFIG, "config entry has invalid type for %s", key);

		return -1;
	}

	*value = json_object_get_boolean(child_obj);

	log_debug("config entry has valid bool value for %s", key);

	return 1;
}

static ssize_t slavery_config_entry_parse_strings(const json_object *obj,
                                                  const char *key,
                                                  const bool required,
                                                  char ***value) {
	json_object *child_obj;
	ssize_t num_strings;
	*value = NULL;

	if (!json_object_object_get_ex(obj, key, &child_obj)) {
		if (!required) {
			log_debug("config entry is missing optional string/string array value for %s", key);

			return 0;
		}

		log_warning(
		    SLAVERY_ERROR_CONFIG, "config entry is missing required string/string array value for %s", key);

		return -1;
	}

	switch (json_object_get_type(child_obj)) {
		case json_type_string:
			*value = malloc(sizeof(char **));
			**value = strdup(json_object_get_string(child_obj));

			num_strings = 1;

			break;

		case json_type_array: {
			array_list *list = json_object_get_array(child_obj);
			*value = malloc(sizeof(char **) * list->size);

			for (size_t i = 0; i < list->size; i++) {
				json_object *grandchild_obj = json_object_array_get_idx(child_obj, i);

				if (json_object_get_type(grandchild_obj) != json_type_string) {
					log_warning(SLAVERY_ERROR_CONFIG, "config entry has invalid type for item in %s array");

					// TODO free

					return -1;
				}

				*value[i] = strdup(json_object_get_string(grandchild_obj));
			}

			num_strings = list->size;

			break;
		}

		default:
			log_warning(SLAVERY_ERROR_CONFIG, "config entry has invalid type for %s", key);

			return -1;
	}

	log_debug("config entry has valid string/string array value for %s", key);

	return num_strings;
}

slavery_config_entry_t *slavery_config_entry_parse(const char *name, const json_object *obj) {
	log_debug("parsing config entry %s", name);

	slavery_config_entry_t *config_entry = malloc(sizeof(slavery_config_entry_t));
	config_entry->name = strdup(name);

	if (slavery_config_entry_parse_string(obj, "description", false, &config_entry->description) < 0) {
		log_warning(SLAVERY_ERROR_CONFIG, "failed to parse config entry for %s", name);

		return NULL;
	}

	if (slavery_config_entry_parse_bool(obj, "enabled", false, &config_entry->enabled) < 0) {
		log_warning(SLAVERY_ERROR_CONFIG, "failed to parse config entry for %s", name);

		return NULL;
	}

	if (slavery_config_entry_parse_bool(obj, "inhibit_cursor", false, &config_entry->inhibit_cursor) < 0) {
		log_warning(SLAVERY_ERROR_CONFIG, "failed to parse config entry for %s", name);

		return NULL;
	}

	if (slavery_config_entry_parse_bool(obj, "do_default", false, &config_entry->do_default) < 0) {
		log_warning(SLAVERY_ERROR_CONFIG, "failed to parse config entry for %s", name);

		return NULL;
	}

	if (slavery_config_entry_parse_strings(obj, "do_command", false, &config_entry->do_command) < 0) {
		log_warning(SLAVERY_ERROR_CONFIG, "failed to parse config entry for %s", name);

		return NULL;
	}

	/*config_entry->buttons = parse_config_item(obj, json_type_array, "buttons", true, NULL);
	config_entry->action = parse_config_item(obj, json_type_string, "action", false, false);
	config_entry->do_keyboard = parse_config_item(obj, json_type_array, "do_keyboard", false, false);
	config_entry->do_mouse = parse_config_item(obj, json_type_array, "do_mouse", false, false);*/

	/*if ((config_entry->action = parse_entry_action(obj)) == SLAVERY_CONFIG_ACTION_UNKNOWN) {
	    log_warning(SLAVERY_ERROR_CONFIG, "config entry has invalid action");

	    free(config_entry->name);

	    return NULL;
	}*/

	return config_entry;
}

slavery_config_t *slavery_config_read(const char *path) {
	log_debug("parsing config file %s", path);

	int fd;
	char *json;
	struct stat stat;
	json_object *obj;
	slavery_config_t *config;
	slavery_config_entry_t *entry;

	if ((fd = open(path, O_RDONLY)) < 0) {
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
	config->num_entries = 0;

	json_object_object_foreach(obj, name, value) {
		if ((entry = slavery_config_entry_parse(name, value)) == NULL) {
			log_warning(SLAVERY_ERROR_CONFIG, "ignoring config entry %s", name);

			continue;
		}

		config->entries =
		    realloc(config->entries, sizeof(slavery_config_entry_t *) * config->num_entries + 1);
		config->entries[config->num_entries++] = entry;
	}

	log_debug("parsed %u config entries", config->num_entries);

	return config;
}

void slavery_config_print(const slavery_config_t *config) {
	printf("num entries: %ld\n", config->num_entries);

	for (size_t entry_index = 0; entry_index < config->num_entries; entry_index++) {}
}
