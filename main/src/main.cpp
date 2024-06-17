#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "UpdateManager.hpp"

static const char *TAG = "main";

extern "C" void app_main() {
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_LOGI(TAG, "Starting OTA update manager");

  UpdateManager::start_ota();
  
  while (true) {
    vTaskDelay(portMAX_DELAY);
  }
}