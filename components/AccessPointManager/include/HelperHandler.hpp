#pragma once

#include <esp_err.h>
#include <esp_http_server.h>

#include <StorageManagerInterface.hpp>

esp_err_t check_file_exist(const char *filename);

esp_err_t set_content_type(httpd_req_t *req, const char *filename);

esp_err_t stream_file(httpd_req_t *req, const char *filename);

esp_err_t mount_fs(char *base_path, char *partition_label);

esp_err_t unpair_device(StorageManagerInterface *storageManager);

esp_err_t factory_reset(StorageManagerInterface *storageManager);

esp_err_t update_firmware(StorageManagerInterface *storageManager);

esp_err_t restart_device();

esp_err_t existProgramMode(StorageManagerInterface *storageManager);

esp_err_t get_accessory_DB_JSON(StorageManagerInterface *storageManager, char *response,
                                size_t response_size);

esp_err_t set_accessory_DB_JSON(StorageManagerInterface *storageManager, const char *new_accessory_json);