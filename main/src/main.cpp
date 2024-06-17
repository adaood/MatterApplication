#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "AccessPoint.hpp"
#include "StorageManager.hpp"

static const char *TAG = "main";

extern "C" void app_main() {
  ESP_ERROR_CHECK(nvs_flash_init());

  StorageManager *storageManager = new StorageManager();
  if (storageManager == nullptr) {
    ESP_LOGE(TAG, "Failed to create StorageManager instance");
    return;
  }

  bool programMode = storageManager->checkProgramMode();

  if (programMode) {
    ESP_LOGI(TAG, "Program mode is set");
  } else {
    ESP_LOGI(TAG, "Program mode is not set");
  }

  AccessPoint *accessPoint = new AccessPoint(storageManager);

  accessPoint->startWebServer();

  while (true) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}