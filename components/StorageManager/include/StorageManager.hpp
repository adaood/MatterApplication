#pragma once

#include <esp_err.h>

#include "StorageManagerInterface.hpp"

/**
 * @brief Manages storage operations using NVS.
 */
class StorageManager : public StorageManagerInterface {
 public:
  /**
   * @brief Constructs a new StorageManager object.
   */
  StorageManager();

  /**
   * @brief Destroys the StorageManager object.
   */
  ~StorageManager() override;

  /**
   * @brief Initializes the storage manager.
   *
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t initialize() override;

  /**
   * @brief Erases all stored data.
   *
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t eraseAllData() override;

  /**
   * @brief Sets the program mode.
   *
   * @param enable If true, enable program mode; if false, disable it.
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t setProgramMode(bool enable) override;

  /**
   * @brief Checks if program mode is enabled.
   *
   * @param[out] isEnabled Pointer to a bool to store the result.
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t isProgramModeEnabled(bool* isEnabled) override;

  /**
   * @brief Sets the device name.
   *
   * @param name Pointer to a char array containing the device name.
   * @param length Length of the device name.
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t setDeviceName(const char* name, size_t length) override;

  /**
   * @brief Gets the device name.
   *
   * @param[out] name Pointer to a char array to store the device name.
   * @param length Maximum length of the device name to be retrieved.
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t getDeviceName(char* name, size_t length) override;

  /**
   * @brief Gets the length of the device name.
   *
   * @param[out] length Pointer to a size_t to store the length.
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t getDeviceNameLength(size_t* length) override;

  /**
   * @brief Sets the accessory JSON configuration.
   *
   * @param json Pointer to a char array containing the JSON configuration.
   * @param length Length of the JSON configuration.
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t setAccessoryJson(const char* json, size_t length) override;

  /**
   * @brief Gets the accessory JSON configuration.
   *
   * @param[out] json Pointer to a char array to store the JSON configuration.
   * @param length Maximum length of the JSON configuration to be retrieved.
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t getAccessoryJson(char* json, size_t length) override;

  /**
   * @brief Gets the length of the accessory JSON configuration.
   *
   * @param[out] length Pointer to a size_t to store the length.
   * @return ESP_OK on success, an error from esp_err_t otherwise.
   */
  esp_err_t getAccessoryJsonLength(size_t* length) override;

 private:
  // Disable copy constructor and assignment operator
  StorageManager(const StorageManager&) = delete;
  StorageManager& operator=(const StorageManager&) = delete;
};
