#pragma once

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
};

uint8_t slavery_hidpp_lookup_feature_id(const slavery_device_t *device, const uint16_t number);
