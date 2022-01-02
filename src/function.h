/**
 * @file
 * @brief Declaration of protocol functions.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 */

#pragma once

/**
 * @brief Encode a function with the software ID to produce request data.
 */
#define slavery_function_encode(function) ((function << 4) | SLAVERY_SOFTWARE_ID)

/**
 * @brief Functions under the 'root' feature.
 */
typedef enum
{
	SLAVERY_FUNCTION_ROOT_GET_FEATURE_INDEX = 0x00,
	SLAVERY_FUNCTION_ROOT_GET_PROTOCOL_VERSION = 0x01
} slavery_function_root_t;

/**
 * @brief Functions under the 'feature set' feature.
 */
typedef enum
{
	SLAVERY_FUNCTION_FEATURE_SET_GET_COUNT = 0x00,
	SLAVERY_FUNCTION_FEATURE_SET_GET_FEATURE_ID = 0x01,
} slavery_function_feature_set_t;

/**
 * @brief Functions under the 'feature info' feature.
 */
typedef enum
{ SLAVERY_FUNCTION_FEATURE_INFO_GET_BASE = 0x00 } slavery_function_feature_info_t;

/**
 * @brief Functions under the 'firmware' feature.
 */
typedef enum
{
	SLAVERY_FUNCTION_FIRMWARE_GET_ENTITIES = 0x00,
	SLAVERY_FUNCTION_FIRMWARE_GET_VERSION = 0x01
} slavery_function_firmware_t;

/**
 * @brief Functions under the 'name/type' feature.
 */
typedef enum
{
	SLAVERY_FUNCTION_NAME_TYPE_GET_NAME_LENGTH = 0x00,
	SLAVERY_FUNCTION_NAME_TYPE_GET_NAME = 0x01,
	SLAVERY_FUNCTION_NAME_TYPE_GET_TYPE = 0x02
} slavery_function_name_type_t;

/**
 * @brief Functions under the 'reset' feature.
 */
typedef enum
{ SLAVERY_FUNCTION_RESET = 0x01 } slavery_function_reset_t;

/**
 * @brief Functions under the 'crypto' feature.
 */
typedef enum
{
	SLAVERY_FUNCTION_CRYPTO_GET_PART_1 = 0x00,
	SLAVERY_FUNCTION_CRYPTO_GET_PART_2 = 0x01,
	SLAVERY_FUNCTION_CRYPTO_GENERATE = 0x02
} slavery_function_crypto_t;

/**
 * @brief Functions under the 'battery' feature.
 */
typedef enum
{ SLAVERY_FUNCTION_BATTERY_GET_PERCENTAGE = 0x00, } slavery_function_battery_t;

/**
 * @brief Functions under the 'host' feature.
 */
typedef enum
{
	SLAVERY_FUNCTION_HOST_GET_INFO = 0x00,
	SLAVERY_FUNCTION_HOST_SET_HOST = 0x01
} slavery_function_host_t;

/**
 * @brief Functions under the 'controls v4' feature.
 */
typedef enum
{
	SLAVERY_FUNCTION_CONTROLS_V4_GET_COUNT = 0x00,
	SLAVERY_FUNCTION_CONTROLS_V4_GET_BUTTON_INFO = 0x01,
	SLAVERY_FUNCTION_CONTROLS_V4_GET_CID_REPORT_INFO = 0x02,
	SLAVERY_FUNCTION_CONTROLS_V4_SET_CID_REPORT_INFO = 0x03
} slavery_function_controls_v4_t;
