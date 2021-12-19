#pragma once

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

typedef enum
{
	SLAVERY_HIDPP_ENTRY_POINT_ROOT = 0x0000,
	SLAVERY_HIDPP_ENTRY_POINT_FEATURE_SET = 0x0001,
	SLAVERY_HIDPP_ENTRY_POINT_FEATURE_INFO = 0x0002,
	SLAVERY_HIDPP_ENTRY_POINT_FIRMWARE = 0x0003,
	SLAVERY_HIDPP_ENTRY_POINT_NAME = 0x0005,
	SLAVERY_HIDPP_ENTRY_POINT_RESET = 0x0020,
	SLAVERY_HIDPP_ENTRY_POINT_CRYPTO = 0x0021,
	SLAVERY_HIDPP_ENTRY_POINT_BATTERY = 0x1000,
	SLAVERY_HIDPP_ENTRY_POINT_HOST = 0x1814,
	SLAVERY_HIDPP_ENTRY_POINT_CONTROLS_V1 = 0x1b00,
	SLAVERY_HIDPP_ENTRY_POINT_CONTROLS_V3 = 0x1b03,
	SLAVERY_HIDPP_ENTRY_POINT_CONTROLS_V4 = 0x1b04
} slavery_hidpp_entry_point_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_ROOT_GET_FEATURE = 0x00,
	SLAVERY_HIDPP_FUNCTION_ROOT_GET_PROTOCOL_VERSION = 0x01
} slavery_hidpp_function_root_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_FEATURE_SET_GET_NUMBER = 0x00,
	SLAVERY_HIDPP_FUNCTION_FEATURE_SET_GET_FEATURE = 0x01,
} slavery_hidpp_function_feature_set_t;

enum E
{ E1 };

typedef enum
{ SLAVERY_HIDPP_FUNCTION_FEATURE_INFO_GET_BASE = 0x00 } slavery_hidpp_function_feature_info_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_FIRMWARE_GET_ENTITIES = 0x00,
	SLAVERY_HIDPP_FUNCTION_FIRMWARE_GET_VERSION = 0x01
} slavery_hidpp_function_firmware_t;

typedef enum
{
	SLAVERY_HIDPP_FUNCTION_NAME_GET_LENGTH = 0x00,
	SLAVERY_HIDPP_FUNCTION_NAME_GET_NAME = 0x01
} slavery_hidpp_function_name_t;

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

typedef enum
{
	SLAVERY_HIDPP_BUTTON_TYPE_BUTTON = 0x01,
	SLAVERY_HIDPP_BUTTON_TYPE_FUNCTION = 0x02,
	SLAVERY_HIDPP_BUTTON_TYPE_HOTKEY = 0x04,
	SLAVERY_HIDPP_BUTTON_TYPE_FUNCTION_TOGGLE = 0x08
} slavery_hidpp_button_type_t;

struct slavery_receiver_t {
	char *devnode;
	char *vendor_id;
	char *product_id;
	char *driver;
	int fd;
};

struct slavery_device_t {
	slavery_receiver_t *receiver;
	uint8_t index;
	char *protocol_version;
	char *name;
	uint8_t num_buttons;
	slavery_button_t **buttons;
};

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

uint8_t slavery_hidpp_lookup_feature_id(const slavery_device_t *device, const uint16_t number);
const char *slavery_hidpp_get_protocol_version(slavery_device_t *device);
const char *slavery_hidpp_get_name(slavery_device_t *device);
uint8_t slavery_hidpp_controls_get_num_buttons(slavery_device_t *device);
slavery_button_t *slavery_hidpp_controls_get_button(slavery_device_t *device, uint8_t button_index);
void slavery_hidpp_controls_button_remap(slavery_button_t *button);
