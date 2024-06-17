#include "StorageManager.hpp"

#include <esp_err.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

static const char *TAG = "StorageManager";

StorageManager::StorageManager() { ESP_LOGI(TAG, "StorageManager instance created"); }

StorageManager::~StorageManager() { ESP_LOGI(TAG, "StorageManager instance destroyed"); }

esp_err_t StorageManager::eraseStorage() {  // nvs_erase_all
  ESP_LOGI(TAG, "Erasing storage");

  nvs_handle_t nvsHandle;

  // Erase NVS
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "NVS not found, nothing to erase");
    return ESP_OK;
  } else {
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS");
      return err;
    }
  }

  err = nvs_erase_all(nvsHandle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to erase NVS namespace");
    nvs_close(nvsHandle);
    return err;
  }

  nvs_commit(nvsHandle);
  nvs_close(nvsHandle);

  ESP_LOGI(TAG, "Storage erased");
  return ESP_OK;
}

esp_err_t StorageManager::eraseDevice() {
  ESP_LOGI(TAG, "Erasing device");

  // Erase NVS
  esp_err_t err = nvs_flash_erase_partition(CONFIG_SM_NVS_PARTITION);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to erase NVS partition");
    return err;
  }

  ESP_LOGI(TAG, "Device erased");
  return ESP_OK;
}

bool StorageManager::checkProgramMode() {
  ESP_LOGI(TAG, "Checking program mode");

  nvs_handle_t nvsHandle;

  // Open NVS namespace
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READONLY, &nvsHandle);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "NVS not found, defaulting to program mode");
    return true;
  } else {
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS");
      return true;
    }
  }

  // Read program mode
  bool programMode;
  err = nvs_get_u8(nvsHandle, CONFIG_SM_NVS_KEY_PROGRAM_MODE, (uint8_t *)&programMode);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "Program mode not found, defaulting to program mode");
    nvs_close(nvsHandle);
    return true;
  } else {
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to read program mode");
      nvs_close(nvsHandle);
      return true;
    }
  }

  nvs_close(nvsHandle);

  ESP_LOGI(TAG, "Program mode: %s", programMode ? "true" : "false");
  return programMode;
}

esp_err_t StorageManager::setProgramMode(bool programMode) {
  ESP_LOGI(TAG, "Setting program mode");

  nvs_handle_t nvsHandle;

  // Open NVS namespace
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "NVS not found, creating new NVS");
    err = nvs_flash_init_partition(CONFIG_SM_NVS_PARTITION);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize NVS partition");
      return err;
    }
    err =
        nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS");
      return err;
    }
  } else {
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS");
      return err;
    }
  }

  // Write program mode
  err = nvs_set_u8(nvsHandle, CONFIG_SM_NVS_KEY_PROGRAM_MODE, programMode);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write program mode");
    nvs_close(nvsHandle);
    return err;
  }

  // Commit changes
  err = nvs_commit(nvsHandle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit changes");
    nvs_close(nvsHandle);
    return err;
  }

  nvs_close(nvsHandle);

  ESP_LOGI(TAG, "Program mode set to: %s", programMode ? "true" : "false");
  return ESP_OK;
}

esp_err_t StorageManager::getAccessoryDB(char *response, size_t response_size) {
  ESP_LOGI(TAG, "Getting accessory DB");

  nvs_handle_t nvsHandle;

  // Open NVS namespace
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READONLY, &nvsHandle);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "NVS not found, nothing to read");
    return ESP_FAIL;
  } else {
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS");
      return err;
    }
  }

  // Read accessory DB
  size_t required_size;
  err = nvs_get_str(nvsHandle, CONFIG_SM_NVS_KEY_ACCESSORY_DB, NULL, &required_size);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "Accessory DB not found, nothing to read");
    nvs_close(nvsHandle);
    return ESP_FAIL;
  } else {
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to read accessory DB");
      nvs_close(nvsHandle);
      return err;
    }
  }

  if (required_size > response_size) {
    ESP_LOGE(TAG, "Response buffer too small");
    nvs_close(nvsHandle);
    return ESP_ERR_NO_MEM;
  }

  err = nvs_get_str(nvsHandle, CONFIG_SM_NVS_KEY_ACCESSORY_DB, response, &required_size);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read accessory DB");
    nvs_close(nvsHandle);
    return err;
  }

  nvs_close(nvsHandle);

  ESP_LOGI(TAG, "Accessory DB read");
  return ESP_OK;
}

esp_err_t StorageManager::setAccessoryDB(const char *newAccessoryDB) {
  ESP_LOGI(TAG, "Setting accessory DB");

  nvs_handle_t nvsHandle;

  // Open NVS namespace
  esp_err_t err =
      nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
  if (err == ESP_ERR_NVS_NOT_FOUND) {
    ESP_LOGI(TAG, "NVS not found, creating new NVS");
    err = nvs_flash_init_partition(CONFIG_SM_NVS_PARTITION);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize NVS partition");
      return err;
    }
    err =
        nvs_open_from_partition(CONFIG_SM_NVS_PARTITION, CONFIG_SM_NVS_NAMESPACE, NVS_READWRITE, &nvsHandle);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS");
      return err;
    }
  } else {
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Failed to open NVS");
      return err;
    }
  }

  // Write accessory DB
  err = nvs_set_str(nvsHandle, CONFIG_SM_NVS_KEY_ACCESSORY_DB, newAccessoryDB);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to write accessory DB");
    nvs_close(nvsHandle);
    return err;
  }

  // Commit changes
  err = nvs_commit(nvsHandle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit changes");
    nvs_close(nvsHandle);
    return err;
  }

  nvs_close(nvsHandle);

  ESP_LOGI(TAG, "Accessory DB set");
  return ESP_OK;
}