#pragma once

#include <esp_err.h>
#include <esp_http_server.h>

#include <StorageManagerInterface.hpp>

class AccessPoint {
  StorageManagerInterface *storageManager;

  // delete the copy constructor and the assignment operator
  AccessPoint(const AccessPoint &) = delete;
  AccessPoint &operator=(const AccessPoint &) = delete;

  static esp_err_t file_read_handler(httpd_req_t *req);
  static esp_err_t command_handler(httpd_req_t *req);
  static esp_err_t accessories_handler(httpd_req_t *req);
  static esp_err_t wifi_handler(httpd_req_t *req);

 public:
  AccessPoint(StorageManagerInterface *storageManager);
  ~AccessPoint();
  esp_err_t startWebServer();
};