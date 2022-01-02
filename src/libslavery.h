/**
 * @file
 * @brief Main libslavery header.
 *
 * @version $(PROJECT_VERSION)
 * @authors $(PROJECT_AUTHORS)
 * @copyright $(PROJECT_COPYRIGHT)
 * @license $(PROJECT_LICENSE)
 *
 * libslavery implements Logitech's HID++ protocol to allow for working with devices such as the MX Master 3
 * mouse. libslavery provides functions for getting/setting metadata, battery state information, and
 * remapping buttons.
 */

#pragma once

#include "utils.h"

#include <stdint.h>
#include <sys/types.h>

/**
 * @brief Opaque type for a Logitech unifying receiver.
 */
typedef struct slavery_receiver_t slavery_receiver_t;

/**
 * @brief Opaque type for a slavery configuration.
 */
typedef struct slavery_config_t slavery_config_t;

/**
 * @brief Opaque type for compatible devices connected to a receiver.
 */
typedef struct slavery_device_t slavery_device_t;

/**
 * @brief Opaque type for a device button.
 */
typedef struct slavery_button_t slavery_button_t;

/**
 * @brief Scans for receivers currently connected.
 *
 * @param receivers The receiver array to fill.
 * @return int The number of receivers found.
 */
int slavery_scan_receivers(slavery_receiver_t **receivers[]);

/**
 * @brief Frees all memory created under the context of an array of receivers.
 *
 * @param receivers Receiver array to free.
 * @param num_receivers Number of receivers in array.
 */
void slavery_receiver_array_free(slavery_receiver_t *receivers[], const ssize_t num_receivers);

/**
 * @brief Frees all memory created under the context of the receiver.
 *
 * @param receiver Receiver to free.
 * @return int 0 on success, < 0 on error.
 */
int slavery_receiver_free(slavery_receiver_t *receiver);

/**
 * @brief Detect compatibile devices connected to the given receiver.
 *
 * @param receiver Receiver to get devices for.
 * @return int 0 on success, < 0 on error.
 */
int slavery_receiver_scan_devices(slavery_receiver_t *receiver);

/**
 * @brief Get the device at the given index from the receiver.
 *
 * @param receiver Receiver devices are attached to.
 * @param device_index Index of the device attached.
 * @return slavery_device_t* Device at the given index, or NULL if no device/error.
 */
slavery_device_t *slavery_receiver_get_device(slavery_receiver_t *receiver, const uint8_t device_index);

/**
 * @brief Reads config file from a file path.
 *
 * @param path Config file path.
 * @return slavery_config_t* Config object.
 */
slavery_config_t *slavery_config_new(const char *path);

/**
 * @brief Free config data.
 *
 * @param config
 */
void slavery_config_free(slavery_config_t *config);

/**
 * @brief Frees memory for an array of devices.
 *
 * @param devices Array of devices to free.
 * @param num_devices Size of device array.
 */
void slavery_device_array_free(slavery_device_t *devices[], const ssize_t num_devices);

/**
 * @brief Set device config.
 *
 * @param device Device to apply config to.
 * @param config Config to apply.
 * @return int 0 on success, < 0 on error.
 */
int slavery_device_set_config(slavery_device_t *device, const slavery_config_t *config);

/**
 * @brief Frees all memory for a device.
 *
 * @param device Device to free.
 */
void slavery_device_free(slavery_device_t *device);
