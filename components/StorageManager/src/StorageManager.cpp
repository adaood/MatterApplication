#include "StorageManager.hpp"

#include <esp_err.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <string.h>

static const char *TAG = "StorageManager";

StorageManager::StorageManager() { ESP_LOGI(TAG, "StorageManager instance created"); }

StorageManager::~StorageManager() { ESP_LOGI(TAG, "StorageManager instance destroyed"); }

esp_err_t StorageManager::initialize() {
  ESP_LOGI(TAG, "Initializing StorageManager");

  // Initialize NVS flash partition
  esp_err_t err = nvs_flash_init_partition(CONFIG_SM_NVS_PARTITION);
  if (err != ESP_OK) {
    switch (err) {
      case ESP_ERR_NVS_NO_FREE_PAGES:
        ESP_LOGE(TAG, "NVS partition has no free pages, erasing and retrying");
        // Try to erase and reinitialize
        err = nvs_flash_erase_partition(CONFIG_SM_NVS_PARTITION);
        if (err == ESP_OK) {
          err = nvs_flash_init_partition(CONFIG_SM_NVS_PARTITION);
        }
        break;
      case ESP_ERR_NVS_NEW_VERSION_FOUND:
        ESP_LOGE(TAG, "NVS partition contains a new version, erasing and retrying");
        // Erase and reinitialize
        err = nvs_flash_erase_partition(CONFIG_SM_NVS_PARTITION);
        if (err == ESP_OK) {
          err = nvs_flash_init_partition(CONFIG_SM_NVS_PARTITION);
        }
        break;
      default:
        ESP_LOGE(TAG, "Failed to initialize NVS flash, error code: %s", esp_err_to_name(err));
        break;
    }
  }

  if (err == ESP_OK) {
    ESP_LOGI(TAG, "StorageManager initialized successfully");
  } else {
    ESP_LOGE(TAG, "StorageManager initialization failed");
  }

  return err;
}

esp_err_t StorageManager::eraseAllData() {
  ESP_LOGI(TAG, "Erasing all Partitions data");

  esp_err_t err = nvs_flash_erase();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to erase all Partitions data: %s", esp_err_to_name(err));
  }

  return err;
}

esp_err_t StorageManager::setProgramMode(bool enable) {
  ESP_LOGI(TAG, "Setting program mode to %s", enable ? "enabled" : "disabled");

  nvs_handle_t handle;
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    return err;
  }

  uint8_t currentMode;
  err = nvs_get_u8(handle, CONFIG_SM_NVS_KEY_PROGRAM_MODE, &currentMode);
  if (err == ESP_ERR_NVS_NOT_FOUND || currentMode != enable) {
    err = nvs_set_u8(handle, CONFIG_SM_NVS_KEY_PROGRAM_MODE, enable);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to set program mode: %s", esp_err_to_name(err));
      nvs_close(handle);
      return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to commit program mode: %s", esp_err_to_name(err));
    }
  }

  nvs_close(handle);
  return err;
}

esp_err_t StorageManager::isProgramModeEnabled(bool *isEnabled) {
  ESP_LOGI(TAG, "Checking if program mode is enabled");

  nvs_handle_t handle;
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    return err;
  }

  uint8_t mode;
  err = nvs_get_u8(handle, CONFIG_SM_NVS_KEY_PROGRAM_MODE, &mode);
  nvs_close(handle);

  if (err == ESP_ERR_NVS_NOT_FOUND) {
    *isEnabled = false;
    err = ESP_OK;
  } else if (err == ESP_OK) {
    *isEnabled = mode;
  } else {
    ESP_LOGE(TAG, "Failed to get program mode: %s", esp_err_to_name(err));
  }

  return err;
}

esp_err_t StorageManager::setDeviceName(const char *name, size_t length) {
  ESP_LOGI(TAG, "Setting device name");

  nvs_handle_t handle;
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    return err;
  }

  err = nvs_set_blob(handle, CONFIG_SM_NVS_KEY_DEVICE_NAME, name, length);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set device name: %s", esp_err_to_name(err));
    nvs_close(handle);
    return err;
  }

  err = nvs_commit(handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit device name: %s", esp_err_to_name(err));
  }

  nvs_close(handle);
  return err;
}

esp_err_t StorageManager::getDeviceName(char *name, size_t length) {
  ESP_LOGI(TAG, "Getting device name");

  nvs_handle_t handle;
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    return err;
  }

  size_t requiredSize = length;
  err = nvs_get_blob(handle, CONFIG_SM_NVS_KEY_DEVICE_NAME, name, &requiredSize);
  nvs_close(handle);

  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "Device name not found");
    return ESP_FAIL;
  } else if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get device name: %s", esp_err_to_name(err));
  }

  return err;
}

esp_err_t StorageManager::getDeviceNameLength(size_t *length) {
  ESP_LOGI(TAG, "Getting device name length");

  nvs_handle_t handle;
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    return err;
  }

  err = nvs_get_blob(handle, CONFIG_SM_NVS_KEY_DEVICE_NAME, NULL, length);
  nvs_close(handle);

  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGE(TAG, "Failed to get device name length: %s", esp_err_to_name(err));
  }

  return err;
}

esp_err_t StorageManager::getAccessoryJson(char *json, size_t length) {
  ESP_LOGI(TAG, "Getting accessory JSON");

  nvs_handle_t handle;
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    return err;
  }

  size_t requiredSize = length;
  err = nvs_get_str(handle, CONFIG_SM_NVS_KEY_ACCESSORY_DB, json, &requiredSize);
  nvs_close(handle);

  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "Accessory JSON not found");
    return ESP_FAIL;
  } else if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to get accessory JSON: %s", esp_err_to_name(err));
  }

  return err;
}

esp_err_t StorageManager::setAccessoryJson(const char *json, size_t length) {
  ESP_LOGI(TAG, "Setting accessory JSON");

  nvs_handle_t handle;
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    return err;
  }

  err = nvs_set_str(handle, CONFIG_SM_NVS_KEY_ACCESSORY_DB, json);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set accessory JSON: %s", esp_err_to_name(err));
    nvs_close(handle);
    return err;
  }

  err = nvs_commit(handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit accessory JSON: %s", esp_err_to_name(err));
  }

  nvs_close(handle);
  return err;
}

esp_err_t StorageManager::getAccessoryJsonLength(size_t *length) {
  ESP_LOGI(TAG, "Getting accessory JSON length");

  nvs_handle_t handle;
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
    return err;
  }

  err = nvs_get_str(handle, CONFIG_SM_NVS_KEY_ACCESSORY_DB, NULL, length);
  nvs_close(handle);

  if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGE(TAG, "Failed to get accessory JSON length: %s", esp_err_to_name(err));
  }

  return err;
}
