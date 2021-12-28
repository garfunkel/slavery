#pragma once

#include "hidpp.h"

#include <threads.h>
#include <unistd.h>

typedef enum
{
	SLAVERY_REPORT_ID_CONTROL_SHORT = 0x10,
	SLAVERY_REPORT_ID_CONTROL_LONG = 0x11,
	SLAVERY_REPORT_ID_EVENT = 0x20
} slavery_report_id_t;

typedef enum
{
	SLAVERY_HIDPP_DEVICE_INDEX_1 = 0x01,
	SLAVERY_HIDPP_DEVICE_INDEX_2 = 0x02,
	SLAVERY_HIDPP_DEVICE_INDEX_3 = 0x03,
	SLAVERY_HIDPP_DEVICE_INDEX_4 = 0x04,
	SLAVERY_HIDPP_DEVICE_INDEX_5 = 0x05,
	SLAVERY_HIDPP_DEVICE_INDEX_6 = 0x06,
	SLAVERY_HIDPP_DEVICE_INDEX_RECEIVER = 0xff
} slavery_hidpp_device_index_t;

#define FEATURE_ID_MAP(FEATURE_ID)                                          \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_ROOT, 0x0000, "root")               \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_FEATURE_SET, 0x0001, "feature_set") \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_FIRMWARE, 0x0003, "firmware")       \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_NAME_TYPE, 0x0005, "name/type")     \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_RESET, 0x0020, "reset")             \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_CRYPTO, 0x0021, "crypto")           \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_BATTERY, 0x1000, "battery")         \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_HOST, 0x1814, "host")               \
	FEATURE_ID(SLAVERY_HIDPP_FEATURE_ID_CONTROLS_V4, 0x1b04, "controls_v4") \
	FEATURE_ID_UNKNOWN(SLAVERY_HIDPP_FEATURE_ID_UNKNOWN, "unknown")

typedef enum
{
#define FEATURE_ID(feature_id_id, feature_id_value, feature_id_string) feature_id_id = feature_id_value,
#define FEATURE_ID_UNKNOWN(feature_id_id, feature_id_string) feature_id_id
	FEATURE_ID_MAP(FEATURE_ID)
#undef FEATURE_ID
#undef FEATURE_ID_UNKNOWN
} slavery_hidpp_feature_id_t;

#pragma weak slavery_hidpp_feature_id_to_string
const char *slavery_hidpp_feature_id_to_string(const slavery_hidpp_feature_id_t feature_id) {
	switch (feature_id) {
#define FEATURE_ID(feature_id_id, feature_id_value, feature_id_string) \
	case feature_id_id:                                                \
		return feature_id_string;
#define FEATURE_ID_UNKNOWN(feature_id_id, feature_id_string) \
	default:                                                 \
		return feature_id_string;
		FEATURE_ID_MAP(FEATURE_ID)
#undef FEATURE_ID
#undef FEATURE_ID_UNKNOWN
#undef FEATURE_ID_MAP
	}
}

typedef enum
{
	SLAVERY_HIDPP_FEATURE_INDEX_ROOT = 0x00,
	SLAVERY_HIDPP_FEATURE_INDEX_ERROR = 0x8f
} slavery_hidpp_feature_index_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_ROOT_GET_FEATURE_INDEX = 0x00,
	SLAVERY_HIDPP_FUNCTION_ROOT_GET_PROTOCOL_VERSION = 0x01
} slavery_hidpp_function_root_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_FEATURE_SET_GET_COUNT = 0x00,
	SLAVERY_HIDPP_FUNCTION_FEATURE_SET_GET_FEATURE_ID = 0x01,
} slavery_hidpp_function_feature_set_t;

typedef enum
{ SLAVERY_HIDPP_FUNCTION_FEATURE_INFO_GET_BASE = 0x00 } slavery_hidpp_function_feature_info_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_FIRMWARE_GET_ENTITIES = 0x00,
	SLAVERY_HIDPP_FUNCTION_FIRMWARE_GET_VERSION = 0x01
} slavery_hidpp_function_firmware_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_NAME_TYPE_GET_NAME_LENGTH = 0x00,
	SLAVERY_HIDPP_FUNCTION_NAME_TYPE_GET_NAME = 0x01,
	SLAVERY_HIDPP_FUNCTION_NAME_TYPE_GET_TYPE = 0x02
} slavery_hidpp_function_name_type_t;

typedef enum
{ SLAVERY_HIDPP_FUNCTION_RESET = 0x01 } slavery_hidpp_function_reset_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_CRYPTO_GET_PART_1 = 0x00,
	SLAVERY_HIDPP_FUNCTION_CRYPTO_GET_PART_2 = 0x01,
	SLAVERY_HIDPP_FUNCTION_CRYPTO_GENERATE = 0x02
} slavery_hidpp_function_crypto_t;

typedef enum
{ SLAVERY_HIDPP_FUNCTION_BATTERY_GET_PERCENTAGE = 0x00, } slavery_hidpp_function_battery_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_HOST_GET_INFO = 0x00,
	SLAVERY_HIDPP_FUNCTION_HOST_SET_HOST = 0x01
} slavery_hidpp_function_host_t;

#define DEVICE_TYPE_MAP(DEVICE_TYPE)                                  \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_KEYBOARD, "keyboard")             \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_REMOTE_CONTROL, "remote_control") \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_NUMPAD, "numpad")                 \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_MOUSE, "mouse")                   \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_TOUCHPAD, "touchpad")             \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_TRACKBALL, "trackball")           \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_PRESENTER, "presenter")           \
	DEVICE_TYPE(SLAVERY_DEVICE_TYPE_RECEIVER, "receiver")             \
	DEVICE_TYPE_UNKNOWN(SLAVERY_DEVICE_TYPE_UNKNOWN, "unknown")

typedef enum
{
#define DEVICE_TYPE(device_type_id, device_type_string) device_type_id,
#define DEVICE_TYPE_UNKNOWN(device_type_id, device_type_string) device_type_id
	DEVICE_TYPE_MAP(DEVICE_TYPE)
#undef DEVICE_TYPE
#undef DEVICE_TYPE_UNKNOWN
} slavery_device_type_t;

#pragma weak slavery_device_type_to_string
const char *slavery_device_type_to_string(const slavery_device_type_t device_type) {
	switch (device_type) {
#define DEVICE_TYPE(device_type_id, device_type_string) \
	case device_type_id:                                \
		return device_type_string;
#define DEVICE_TYPE_UNKNOWN(device_type_id, device_type_string) \
	default:                                                    \
		return device_type_string;
		DEVICE_TYPE_MAP(DEVICE_TYPE)
#undef DEVICE_TYPE
#undef DEVICE_TYPE_UNKNOWN
#undef DEVICE_TYPE_MAP
	}
}

typedef enum
{
	SLAVERY_HIDPP_BUTTON_TYPE_BUTTON = 0x01,
	SLAVERY_HIDPP_BUTTON_TYPE_FUNCTION = 0x02,
	SLAVERY_HIDPP_BUTTON_TYPE_HOTKEY = 0x04,
	SLAVERY_HIDPP_BUTTON_TYPE_FUNCTION_TOGGLE = 0x08
} slavery_hidpp_button_type_t;

typedef struct {
	uint16_t id;
	uint8_t index;
	uint8_t version;
	uint8_t flags;
} slavery_feature_t;

struct slavery_button_t {
	slavery_device_t *device;
	uint8_t index;
	uint16_t cid;
	uint16_t task_id;
	uint8_t flags;
	bool virtual;
	bool persistent_divert;
	bool temporary_divert;
	bool reprogrammable;
	slavery_hidpp_button_type_t type;
	uint8_t function_position;
	uint8_t group;
	uint8_t group_remap_mask;
	bool gesture;
};

typedef struct slavery_event_t {
	slavery_receiver_t *receiver;
	size_t request_size;
	size_t response_size;
	uint8_t *request_data;
	uint8_t *response_data;
} slavery_event_t;

struct slavery_receiver_t {
	char *devnode;
	uint16_t vendor_id;
	uint16_t product_id;
	char *name;
	char *address;
	size_t num_devices;
	slavery_device_t **devices;
	thrd_t listener_thread;
	int fd;
	int control_pipe[2];
};

struct slavery_device_t {
	slavery_receiver_t *receiver;
	uint8_t index;
	char *protocol_version;
	slavery_device_type_t type;
	char *name;
	size_t num_features;
	slavery_feature_t **features;
	size_t num_buttons;
	slavery_button_t **buttons;
};

extern const uint16_t SLAVERY_USB_VENDOR_ID_LOGITECH;
extern const uint16_t SLAVERY_USB_PRODUCT_ID_UNIFYING_RECEIVER;

extern const size_t SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_SHORT;
extern const size_t SLAVERY_HIDPP_PACKET_LENGTH_CONTROL_LONG;
extern const size_t SLAVERY_HIDPP_PACKET_LENGTH_EVENT;
extern const size_t SLAVERY_HIDPP_PACKET_LENGTH_MAX;

extern const uint8_t SLAVERY_HIDPP_SOFTWARE_ID;
extern const uint8_t SLAVERY_HIDPP_FEATURE_ROOT;
extern const uint8_t SLAVERY_HIDPP_FEATURE_ERROR;

ssize_t slavery_hidpp_get_features(slavery_device_t *device);
uint8_t slavery_hidpp_feature_count(slavery_device_t *device);
ssize_t slavery_hidpp_feature_id_to_index(slavery_device_t *device, const uint16_t id);
const char *slavery_hidpp_get_protocol_version(slavery_device_t *device);
slavery_device_type_t slavery_hidpp_get_type(slavery_device_t *device);
const char *slavery_hidpp_get_name(slavery_device_t *device);
ssize_t slavery_hidpp_controls_get_num_buttons(slavery_device_t *device);
slavery_button_t *slavery_hidpp_controls_get_button(slavery_device_t *device, uint8_t button_index);
void slavery_hidpp_controls_button_remap(slavery_button_t *button);

slavery_receiver_t *slavery_receiver_from_devnode(const char *devnode);
int slavery_receiver_get_report_descriptor(slavery_receiver_t *receiver);
void *slavery_receiver_listen(slavery_receiver_t *receiver);
int slavery_receiver_control_read_response(slavery_receiver_t *receiver,
                                           uint8_t response_data[],
                                           ssize_t response_size);
int slavery_receiver_control_write_response(slavery_receiver_t *receiver,
                                            uint8_t response_data[],
                                            ssize_t response_size);

void *slavery_event_dispatch(slavery_event_t *event);

#define slavery_hidpp_encode_function(function) ((function << 4) | SLAVERY_HIDPP_SOFTWARE_ID)
