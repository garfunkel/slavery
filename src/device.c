/**
 * @file
 * @brief Device functions and types.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#define _GNU_SOURCE

#include "device.h"

#include "button.h"
#include "feature.h"
#include "function.h"
#include "receiver.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void slavery_device_array_free(slavery_device_t *devices[], const ssize_t num_devices) {
	log_debug("freeing device array at %p", devices);

	for (ssize_t i = 0; i < num_devices; i++) {
		slavery_device_free(devices[i]);
	}

	free(devices);
}

void slavery_device_free(slavery_device_t *device) {
	log_debug("freeing device %s:%u...", device->receiver->devnode, device->index);

	free(device->protocol_version);
	free(device->name);

	for (size_t i = 0; i < device->num_buttons; i++) {
		free(device->buttons[i]);
	}

	for (size_t i = 0; i < device->num_features; i++) {
		free(device->features[i]);
	}

	free(device->buttons);
	free(device->features);
	free(device);
}

int slavery_device_set_config(slavery_device_t *device, const slavery_config_t *config) {
	UNUSED(device);
	UNUSED(config);

	return 0;
}

ssize_t slavery_device_get_features(slavery_device_t *device) {
	log_debug("getting features for device %s:%u...", device->receiver->devnode, device->index);
	log_debug("checking if device exists");

	device->features = NULL;
	device->num_features = 0;
	slavery_feature_t *feature;

	// Get root feature, which we already know the details of, but lets do it for completeness.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_ROOT)) == NULL) {
		log_debug("couldn't get root feature for device %s:%u", device->receiver->devnode, device->index);

		return -1;
	}

	device->features = realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
	device->features[device->num_features++] = feature;

	// Get feature set feature.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_FEATURE_SET)) == NULL) {
		log_debug(
		    "couldn't get feature set feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get firmware feature.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_FIRMWARE)) == NULL) {
		log_debug("couldn't get firmware feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get name feature.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_NAME_TYPE)) == NULL) {
		log_debug("couldn't get name feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get reset feature.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_RESET)) == NULL) {
		log_debug("couldn't get reset feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get crypto feature.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_CRYPTO)) == NULL) {
		log_debug("couldn't get crypto feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get battery feature.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_BATTERY)) == NULL) {
		log_debug("couldn't get battery feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get host feature.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_HOST)) == NULL) {
		log_debug("couldn't get host feature for device %s:%u", device->receiver->devnode, device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	// Get controls v4 feature.
	if ((feature = slavery_device_get_feature(device, SLAVERY_FEATURE_ID_CONTROLS_V4)) == NULL) {
		log_debug("couldn't get controls v4 set feature for device %s:%u",
		          device->receiver->devnode,
		          device->index);
	} else {
		device->features =
		    realloc(device->features, sizeof(slavery_feature_t *) * (device->num_features + 1));
		device->features[device->num_features++] = feature;
	}

	log_debug("found %d features for device %s:%u...",
	          device->num_features,
	          device->receiver->devnode,
	          device->index);

	return device->num_features;
}

slavery_feature_t *slavery_device_get_feature(slavery_device_t *device, slavery_feature_id_t feature_id) {
	log_debug("getting feature information for feature %s", slavery_feature_id_to_string(feature_id));

	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          SLAVERY_FEATURE_INDEX_ROOT,
	                          slavery_function_encode(SLAVERY_FUNCTION_ROOT_GET_FEATURE_INDEX),
	                          feature_id >> 8,
	                          feature_id & 0xff,
	                          0x00};
	uint8_t response_data[SLAVERY_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request feature");

		return NULL;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return NULL;
	}

	if (response_data[4] == 0x00 && response_data[5] == 0x00 && response_data[6] == 0x00) {
		log_debug("feature doesn't exist");

		return NULL;
	}

	slavery_feature_t *feature = malloc(sizeof(slavery_feature_t));
	feature->id = feature_id;
	feature->index = response_data[4];
	feature->flags = response_data[5];
	feature->version = response_data[6];

	log_debug("received information for feature %s", slavery_feature_id_to_string(feature_id));

	return feature;
}

const char *slavery_device_get_protocol_version(slavery_device_t *device) {
	log_debug("getting protocol version for device %s:%u...", device->receiver->devnode, device->index);

	device->protocol_version = NULL;
	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          SLAVERY_FEATURE_INDEX_ROOT,
	                          slavery_function_encode(SLAVERY_FUNCTION_ROOT_GET_PROTOCOL_VERSION),
	                          0x00,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request device protocol");

		return NULL;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return NULL;
	}

	asprintf(&device->protocol_version, "%d.%d", response_data[4], response_data[5]);

	log_debug("device protocol %s", device->protocol_version);

	return device->protocol_version;
}

slavery_device_type_t slavery_device_get_type(slavery_device_t *device) {
	log_debug("getting type for device %s:%u...", device->receiver->devnode, device->index);

	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          slavery_feature_id_to_index(device, SLAVERY_FEATURE_ID_NAME_TYPE),
	                          slavery_function_encode(SLAVERY_FUNCTION_NAME_TYPE_GET_TYPE),
	                          0x00,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request type");

		return -1;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return SLAVERY_DEVICE_TYPE_UNKNOWN;
	}

	device->type = response_data[4];

	log_debug("device type %s", slavery_device_type_to_string(device->type));

	return device->type;
}

const char *slavery_device_get_name(slavery_device_t *device) {
	log_debug("getting name length for device %s:%u...", device->receiver->devnode, device->index);

	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          slavery_feature_id_to_index(device, SLAVERY_FEATURE_ID_NAME_TYPE),
	                          slavery_function_encode(SLAVERY_FUNCTION_NAME_TYPE_GET_NAME_LENGTH),
	                          0x00,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request name length");

		return NULL;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return NULL;
	}

	log_debug("device name length: %u", response_data[4]);

	uint8_t name_length = response_data[4];
	char *name = malloc(name_length + 1);
	char *name_pos = name;
	request_data[3] = slavery_function_encode(SLAVERY_FUNCTION_NAME_TYPE_GET_NAME);

	log_debug("getting name for device %s:%u...", device->receiver->devnode, device->index);

	do {
		if (write(device->receiver->fd, request_data, SLAVERY_PACKET_LENGTH_CONTROL_SHORT) !=
		    (ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_SHORT) {
			log_warning_errno(SLAVERY_ERROR_IO, "failed to request name");

			return NULL;
		}

		if (slavery_receiver_control_read_response(
		        device->receiver, response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG) < 0) {
			log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

			return NULL;
		}

		ssize_t num_bytes = (name + name_length + 1) - name_pos;

		if (num_bytes > 16) {
			num_bytes = 16;
		}

		name_pos = stpncpy(name_pos, (const char *)response_data + 4, num_bytes);
		request_data[4] += num_bytes;
	} while (request_data[4] < name_length);

	device->name = name;

	log_debug("device name: %s", device->name);

	return device->name;
}

ssize_t slavery_device_get_num_buttons(slavery_device_t *device) {
	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          slavery_feature_id_to_index(device, SLAVERY_FEATURE_ID_CONTROLS_V4),
	                          slavery_function_encode(SLAVERY_FUNCTION_CONTROLS_V4_GET_COUNT),
	                          0x00,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_PACKET_LENGTH_CONTROL_LONG];

	if (write(device->receiver->fd, request_data, SLAVERY_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_SHORT) {
		log_warning_errno(SLAVERY_ERROR_IO, "failed to request number of buttons");

		return -1;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return -1;
	}

	device->num_buttons = response_data[4];

	return device->num_buttons;
}

slavery_button_t *slavery_device_get_button(slavery_device_t *device, uint8_t button_index) {
	uint8_t request_data[] = {SLAVERY_REPORT_ID_CONTROL_SHORT,
	                          device->index,
	                          slavery_feature_id_to_index(device, SLAVERY_FEATURE_ID_CONTROLS_V4),
	                          slavery_function_encode(SLAVERY_FUNCTION_CONTROLS_V4_GET_BUTTON_INFO),
	                          button_index,
	                          0x00,
	                          0x00};
	uint8_t response_data[SLAVERY_PACKET_LENGTH_CONTROL_LONG];
	slavery_button_t *button;

	if (write(device->receiver->fd, request_data, SLAVERY_PACKET_LENGTH_CONTROL_SHORT) !=
	    (ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_SHORT) {
		return NULL;
	}

	if (slavery_receiver_control_read_response(
	        device->receiver, response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG) < 0) {
		log_warning(SLAVERY_ERROR_IO, "failed to read control event response from device");

		return NULL;
	}

	button = malloc(sizeof(slavery_button_t));

	printf("get button info: ");

	for (size_t i = 0; i < SLAVERY_PACKET_LENGTH_CONTROL_LONG; i++) {
		printf("%hhx ", response_data[i]);
	}

	puts("");

	button->index = button_index;
	button->device = device;
	button->cid = (slavery_cid_t)response_data[4] << 8 | response_data[5];
	button->task_id = (uint16_t)response_data[6] << 8 | response_data[7];
	button->flags = response_data[8];
	button->virtual = response_data[8] & 0x80;
	button->persistent_divert = response_data[8] & 0x40;
	button->temporary_divert = response_data[8] & 0x20;
	button->reprogrammable = response_data[8] & 0x10;
	button->type = (slavery_button_type_t)button->flags & 0x0f;
	button->function_position = response_data[9];
	button->group = response_data[10];
	button->group_remap_mask = response_data[11];
	button->gesture = response_data[12];

	printf("index: %u\n", button->index);
	printf("cid: %d\n", button->cid);
	printf("task id: %u\n", button->task_id);
	printf("virtual: %d\n", button->virtual);
	printf("persistent divert: %d\n", button->persistent_divert);
	printf("temporary divert: %d\n", button->temporary_divert);
	printf("reprogrammable: %d\n", button->reprogrammable);
	printf("fn toggle: %d\n", button->type == SLAVERY_BUTTON_TYPE_FUNCTION_TOGGLE);
	printf("hotkey: %d\n", button->type == SLAVERY_BUTTON_TYPE_HOTKEY);
	printf("fn: %d\n", button->type == SLAVERY_BUTTON_TYPE_FUNCTION);
	printf("mouse: %d\n", button->type == SLAVERY_BUTTON_TYPE_BUTTON);
	printf("fn positon: %d\n", button->function_position);
	printf("group: %d\n", button->group);
	printf("group remap mask: %d\n", button->group_remap_mask);
	printf("gesture: %d\n", button->gesture);
	puts("");

	device->buttons[button_index] = button;

	return device->buttons[button_index];
}

/*
none = 0x33
*/
void slavery_device_remap_button(slavery_device_t *device, slavery_button_t *button) {
	UNUSED(device);
	UNUSED(button);

	/*uint8_t request_data[] = {
	    SLAVERY_REPORT_ID_CONTROL_LONG,
	    button->device->index,
	    slavery_feature_id_to_index(button->device, SLAVERY_FEATURE_ID_CONTROLS_V4),
	    slavery_function_encode(SLAVERY_FUNCTION_CONTROLS_V4_SET_CID_REPORT_INFO),
	    button->cid >> 8,
	    button->cid & 0xff,
	    / *
	    2 mouse move
	    3 stop mouse move

	    a remap
	    3 stop map

	    mapped to back:
	    event size: 15: 20 01 02 08 00 00 00 00 00 00 36 00 00 00 00
	    event size: 15: 20 01 02 00 00 00 00 00 00 00 3e 00 00 00 00

	    no map:
	    event size: 20: 11 01 09 00 00 c3 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	    event size: 20: 11 01 09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

	    back:
	    event size: 15: 20 01 02 08 00 00 00 00 00 00 36 00 00 00 00
	    event size: 15: 20 01 02 00 00 00 00 00 00 00 3e 00 00 00 00
	    * /
	0x2a,
	0x00, // button->cid >> 8,
	0x53, // button->cid & 0xff,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00};
uint8_t response_data[SLAVERY_PACKET_LENGTH_CONTROL_LONG];

printf("reprog: %d, pers: %d, temp: %d\n",
	   button->reprogrammable,
	   button->persistent_divert,
	   button->temporary_divert);
printf("set button remap: ");

for (size_t i = 0; i < SLAVERY_PACKET_LENGTH_CONTROL_LONG; i++) {
	printf("%hhx ", request_data[i]);
}

puts("");

if (write(button->device->receiver->fd, request_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG) !=
	(ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_LONG) {
	return;
}

if (read(button->device->receiver->control_pipe[0], response_data, SLAVERY_PACKET_LENGTH_CONTROL_LONG)
!= (ssize_t)SLAVERY_PACKET_LENGTH_CONTROL_LONG) { return;
}

if (response_data[2] == 0xff) {
	printf("remap error\n");

	exit(0);

	return;
}

for (size_t i = 0; i < SLAVERY_PACKET_LENGTH_CONTROL_LONG; i++) {
	printf("%hhx ", response_data[i]);
}

puts("");
*/
}

ssize_t slavery_feature_id_to_index(slavery_device_t *device, const uint16_t id) {
	for (size_t i = 0; i < device->num_features; i++) {
		if (device->features[i]->id == id) {
			return device->features[i]->index;
		}
	}

	return -1;
}
