#include "UpdateManager.hpp"

#include <esp_err.h>
#include <esp_event.h>
#include <esp_https_ota.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "UpdateManager";
extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");

void UpdateManager::ota_task(void *pvParameter) {
  ESP_LOGI(TAG, "Starting OTA task");

  esp_err_t ota_finish_err = ESP_OK;
  esp_http_client_config_t config = {
      .url = CONFIG_UM_FIRMWARE_UPGRADE_URL,
      .cert_pem = (char *)server_root_cert_pem_start,
      .cert_len = strlen((char *)server_root_cert_pem_start) + 1,
      // .client_cert_pem = (char *)server_root_cert_pem_start,
      // .client_cert_len = strlen((char *)server_root_cert_pem_start) + 1,
      .timeout_ms = CONFIG_UM_OTA_RECV_TIMEOUT * 5,
      .is_async = false,
      .skip_cert_common_name_check = true,
      .keep_alive_enable = true,
  };

  esp_https_ota_config_t ota_config = {
      .http_config = &config,
      .partial_http_download = true,
      .max_http_request_size = CONFIG_UM_HTTP_REQUEST_SIZE,
  };

  esp_https_ota_handle_t https_ota_handle = NULL;
  esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
  if (err != ESP_OK) {
    // print the error and delete the task name
    ESP_LOGE(TAG, "esp_https_ota_begin failed, error=%s", esp_err_to_name(err));

    vTaskDelete(NULL);
  }

  esp_app_desc_t app_desc;
  err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_https_ota_get_img_desc failed");
    goto ota_end;
  }
  err = validate_image_header(&app_desc);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "image header verification failed");
    goto ota_end;
  }

  while (1) {
    err = esp_https_ota_perform(https_ota_handle);
    if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
      break;
    }
    // esp_https_ota_perform returns after every read operation which gives user the ability to
    // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of
    // image data read so far.
    ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
  }

  if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
    // the OTA image was not completely received and user can customise the response to this situation.
    ESP_LOGE(TAG, "Complete data was not received.");
  } else {
    ota_finish_err = esp_https_ota_finish(https_ota_handle);
    if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
      ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      esp_restart();
    } else {
      if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
        ESP_LOGE(TAG, "Image validation failed, image is corrupted");
      }
      ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
      vTaskDelete(NULL);
    }
  }

ota_end:
  esp_https_ota_abort(https_ota_handle);
  ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
  vTaskDelete(NULL);
}

esp_err_t UpdateManager::validate_image_header(esp_app_desc_t *new_app_info) {
  if (new_app_info == NULL) {
    return ESP_ERR_INVALID_ARG;
  }

  const esp_partition_t *running = esp_ota_get_running_partition();
  esp_app_desc_t running_app_info;
  if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
    ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
  }

  if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0) {
    ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
    return ESP_FAIL;
  }

  return ESP_OK;
}

void UpdateManager::event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,
                                  void *event_data) {
  if (event_base == ESP_HTTPS_OTA_EVENT) {
    switch (event_id) {
      case ESP_HTTPS_OTA_START:
        ESP_LOGI(TAG, "OTA started");
        break;
      case ESP_HTTPS_OTA_CONNECTED:
        ESP_LOGI(TAG, "Connected to server");
        break;
      case ESP_HTTPS_OTA_GET_IMG_DESC:
        ESP_LOGI(TAG, "Reading Image Description");
        break;
      case ESP_HTTPS_OTA_VERIFY_CHIP_ID:
        ESP_LOGI(TAG, "Verifying chip id of new image: %d", *(esp_chip_id_t *)event_data);
        break;
      case ESP_HTTPS_OTA_DECRYPT_CB:
        ESP_LOGI(TAG, "Callback to decrypt function");
        break;
      case ESP_HTTPS_OTA_WRITE_FLASH:
        ESP_LOGD(TAG, "Writing to flash: %d written", *(int *)event_data);
        break;
      case ESP_HTTPS_OTA_UPDATE_BOOT_PARTITION:
        ESP_LOGI(TAG, "Boot partition updated. Next Partition: %d", *(esp_partition_subtype_t *)event_data);
        break;
      case ESP_HTTPS_OTA_FINISH:
        ESP_LOGI(TAG, "OTA finish");
        break;
      case ESP_HTTPS_OTA_ABORT:
        ESP_LOGI(TAG, "OTA abort");
        break;
    }
  }

  if (event_base == WIFI_EVENT) {
    switch (event_id) {
      // case WIFI_EVENT_STA_START:
      //   esp_wifi_connect();
      //   break;
      case WIFI_EVENT_STA_CONNECTED:
        wifi_connected = true;
        break;
      case WIFI_EVENT_STA_DISCONNECTED:
        wifi_connected = false;
        break;
    }
  }
}

void UpdateManager::start_ota() {
  ESP_LOGI("UpdateManager", "start_ota");
  esp_netif_init();
  esp_event_loop_create_default();

  esp_event_handler_register(ESP_HTTPS_OTA_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);
  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL);

  esp_netif_create_default_wifi_sta();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_start();

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = CONFIG_UM_WIFI_SSID,
              .password = CONFIG_UM_WIFI_PASSWORD,
              .scan_method = WIFI_ALL_CHANNEL_SCAN,
              .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
          },
  };
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

  wifi_connected = false;
  uint32_t retry = 0;

  esp_wifi_connect();

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(100));
    retry += 100;
    if (wifi_connected == true) {
      ESP_LOGI(TAG, "Connected to WiFi");
      break;
    }
    if ((retry) >= (CONFIG_UM_WIFI_CONNECT_TIMEOUT * 1000)) {
      ESP_LOGW(TAG, "Failed to connect to WiFi");
      return;
      break;
    }
  }

  vTaskDelay(pdMS_TO_TICKS(10000));

  xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
}
