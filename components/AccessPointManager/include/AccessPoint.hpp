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

  // callback function for the wifi event
  static void change_led_status(int32_t event_id);  /// TODO : Reimplement this function

  /**  event handler for wifi events
   *   to handle the wifi events and change the led status when WIFI_EVENT_AP_STACONNECTED and
   * WIFI_EVENT_AP_STADISCONNECTED
   */
  static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

 public:
  AccessPoint(StorageManagerInterface *storageManager);
  ~AccessPoint();
  esp_err_t startWebServer();
};