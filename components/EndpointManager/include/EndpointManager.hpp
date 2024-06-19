#pragma once

#include <esp_err.h>
#include <esp_matter.h>

class EndpointManager {
 private:
  esp_matter::endpoint_t *node;
  esp_matter::endpoint_t *aggregator;

  // delete the copy constructor and the assignment operator
  EndpointManager(const EndpointManager &) = delete;
  EndpointManager &operator=(const EndpointManager &) = delete;

  static esp_err_t app_identification_cb(esp_matter::identification::callback_type type, uint16_t endpoint_id,
                                         uint8_t effect_id, uint8_t effect_variant, void *priv_data);

  static esp_err_t app_attribute_cb(esp_matter::attribute::callback_type type, uint16_t endpoint_id,
                                    uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val,
                                    void *priv_data);

  static void app_event_cb(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg);

 public:
  EndpointManager(bool isBridge = false);
  ~EndpointManager();

  esp_err_t createArrayOfEndpoints(const char *jsonArray, size_t jsonArraySize);

  esp_err_t startMatter();
};