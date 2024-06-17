#pragma once

#include <esp_app_desc.h>
#include <esp_err.h>
#include <esp_event.h>

class UpdateManager {
 public:
  static void ota_task(void* pvParameter);
  static esp_err_t validate_image_header(esp_app_desc_t* new_app_info);
  static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
  static void start_ota();
};