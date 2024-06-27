#include "EndpointCreator.hpp"

#include <esp_log.h>

static const char* TAG = "EndpointCreator";

static const ConfigItem m_lightConfigItems[] = {
    {"name", "string"}, {"lightPin", "number"}, {"buttonPin", "number"}};

static const ConfigItem m_fanConfigItems[] = {
    {"name", "string"}, {"fanPin", "number"}, {"buttonPin", "number"}};

static const ConfigItem m_pluginConfigItems[] = {
    {"name", "string"}, {"pluginPin", "number"}, {"buttonPin", "number"}};

static const ConfigItem m_buttonConfigItems[] = {{"name", "string"}, {"buttonPin", "number"}};

static const ConfigItem m_windowConfigItems[] = {{"name", "string"},          {"motorUpPin", "number"},
                                                 {"motorDownPin", "number"},  {"buttonUpPin", "number"},
                                                 {"buttonDownPin", "number"}, {"timeToOpen", "number"},
                                                 {"timeToClose", "number"}};

static const DeviceType m_deviceTypes[] = {{"LIGHT", DeviceCreator::createLight},
                                           {"FAN", DeviceCreator::createFan},
                                           {"PLUGIN", DeviceCreator::createPlugin},
                                           {"BUTTON", DeviceCreator::createButton},
                                           {"WINDOW", DeviceCreator::createWindow}};

void DeviceCreator::getJsonSchemaForAllDevices(char* jsonSchema, size_t schemaSize) {};

void DeviceCreator::getJsonSchemaSizeForAllDevices(size_t* schemaSize) {};

BaseDeviceInterface* DeviceCreator::createDevice(JsonObject deviceJson, esp_matter::endpoint_t* aggregator) {
  const char* type = deviceJson["type"].as<const char*>();

  for (int i = 0; i < sizeof(m_deviceTypes) / sizeof(DeviceType); i++) {
    if (strcmp(type, m_deviceTypes[i].type) == 0) {
      return m_deviceTypes[i].createFunction(deviceJson, aggregator);
    }
  }
  ESP_LOGE(TAG, "Device type not found: %s", type);
  return nullptr;
}

BaseDeviceInterface* DeviceCreator::createLight(JsonObject deviceJson, esp_matter::endpoint_t* aggregator) {}