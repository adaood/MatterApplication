#include "HelperHandler.hpp"

#include <esp_err.h>
#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_spiffs.h>

#define IS_FILE_EXT(filename, ext) (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

esp_err_t check_file_exist(const char *filename) {
  // Check if the file exists
  if (!filename) {
    return ESP_ERR_INVALID_ARG;
  }

  FILE *file = fopen(filename, "r");
  if (!file) {
    return ESP_ERR_NOT_FOUND;
  }

  fclose(file);
  return ESP_OK;
}

esp_err_t set_content_type(httpd_req_t *req, const char *filename) {
  if (!filename || !req) {
    return ESP_ERR_INVALID_ARG;
  }
  if (IS_FILE_EXT(filename, ".htm")) return httpd_resp_set_type(req, "text/html");
  if (IS_FILE_EXT(filename, ".html")) return httpd_resp_set_type(req, "text/html");
  if (IS_FILE_EXT(filename, ".css")) return httpd_resp_set_type(req, "text/css");
  if (IS_FILE_EXT(filename, ".js")) return httpd_resp_set_type(req, "application/javascript");
  if (IS_FILE_EXT(filename, ".png")) return httpd_resp_set_type(req, "image/png");
  if (IS_FILE_EXT(filename, ".gif")) return httpd_resp_set_type(req, "image/gif");
  if (IS_FILE_EXT(filename, ".jpg")) return httpd_resp_set_type(req, "image/jpeg");
  if (IS_FILE_EXT(filename, ".ico")) return httpd_resp_set_type(req, "image/x-icon");
  if (IS_FILE_EXT(filename, ".xml")) return httpd_resp_set_type(req, "text/xml");
  if (IS_FILE_EXT(filename, ".pdf")) return httpd_resp_set_type(req, "application/x-pdf");
  if (IS_FILE_EXT(filename, ".zip")) return httpd_resp_set_type(req, "application/x-zip");
  if (IS_FILE_EXT(filename, ".gz")) return httpd_resp_set_type(req, "application/x-gzip");

  return httpd_resp_set_type(req, "text/plain");
}

esp_err_t stream_file(httpd_req_t *req, const char *filename) {
  if (!filename || !req) {
    return ESP_ERR_INVALID_ARG;
  }

  // Open the file
  FILE *file = fopen(filename, "r");
  if (!file) {
    return ESP_FAIL;
  }

  // Stream the file
  char buffer[CONFIG_AP_STREAM_BUFFER_SIZE];
  size_t read_bytes;
  do {
    read_bytes = fread(buffer, 1, CONFIG_AP_STREAM_BUFFER_SIZE, file);
    ESP_LOGI("stream_file", "Read %zu bytes", read_bytes);
    if (read_bytes > 0) {
      esp_err_t err = httpd_resp_send_chunk(req, buffer, read_bytes);
      if (err != ESP_OK) {
        fclose(file);
        /* Abort sending file */
        httpd_resp_sendstr_chunk(req, NULL);  // TODO:  httpd_resp_send_chunk(req, NULL, 0);
        return err;
      }
    }
  } while (read_bytes > 0);

  // Close the file
  fclose(file);

  // Finish the response
  return httpd_resp_send_chunk(req, NULL, 0);
}

esp_err_t mount_fs(char *base_path, char *partition_label) {
  if (!base_path || !partition_label) {
    return ESP_ERR_INVALID_ARG;
  }

  esp_vfs_spiffs_conf_t conf = {
      .base_path = base_path,
      .partition_label = partition_label,
      .max_files = 5,  // This sets the maximum number of files that can be open at the same time
      .format_if_mount_failed = false};

  esp_err_t err = esp_vfs_spiffs_register(&conf);
  if (err != ESP_OK) {
    return err;
  }

  return ESP_OK;
}

esp_err_t unpair_device(StorageManagerInterface *storageManager) {
  // get the accessory DB from the storageManager
  char response[CONFIG_AP_ACCESSORY_JSON_SIZE];
  storageManager->getAccessoryDB(response, sizeof(response));
  ESP_LOGI("unpair_device", "Accessory DB: %s", response);

  // erase the storage
  storageManager->eraseStorage();

  // set the accessory DB to the storageManager
  storageManager->setAccessoryDB(response);

  // restart the device
  restart_device();

  return ESP_OK;
}

esp_err_t factory_reset(StorageManagerInterface *storageManager) {
  // erase the device
  storageManager->eraseDevice();

  // restart the device
  restart_device();

  return ESP_OK;
}

esp_err_t update_firmware(StorageManagerInterface *storageManager) { return ESP_OK; }

esp_err_t restart_device() {
  esp_restart();
  return ESP_OK;
}

esp_err_t existProgramMode(StorageManagerInterface *storageManager) {
  storageManager->setProgramMode(false);

  return ESP_OK;
}

esp_err_t get_accessory_DB_JSON(StorageManagerInterface *storageManager, char *response,
                                size_t response_size) {
  storageManager->getAccessoryDB(response, response_size);

  return ESP_OK;
}

esp_err_t set_accessory_DB_JSON(StorageManagerInterface *storageManager, const char *new_accessory_json) {
  storageManager->setAccessoryDB(new_accessory_json);

  return ESP_OK;
}