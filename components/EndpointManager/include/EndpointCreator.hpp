#pragma once

#include <ArduinoJson.h>
#include <esp_matter.h>

#include <BaseDeviceInterface.hpp>
#include <ButtonDevice.hpp>
#include <FanDevice.hpp>
#include <LightDevice.hpp>
#include <PluginDevice.hpp>
#include <WindowDevice.hpp>

/**
 * @brief Typedef for the function pointer used to create devices.
 */
using CreateFunction = BaseDeviceInterface* (*)(JsonObject deviceJson, esp_matter::endpoint_t* aggregator);

/**
 * @brief Structure representing a device type.
 */
struct DeviceType {
  const char* type;              /**< Type of the device. */
  CreateFunction createFunction; /**< Function to create the device. */
};

/**
 * @brief Class for creating devices based on JSON input.
 */
class DeviceCreator {
 public:
  DeviceCreator() = default;
  ~DeviceCreator() = default;

  /**
   * @brief Get the JSON schema for all devices.
   * @param jsonSchema Pointer to the JSON schema buffer.
   * @param schemaSize Size of the JSON schema buffer.
   */
  void getJsonSchemaForAllDevices(char* jsonSchema, size_t schemaSize);

  /**
   * @brief Get the size of the JSON schema for all devices.
   * @param schemaSize Pointer to the size of the JSON schema.
   */
  void getJsonSchemaSizeForAllDevices(size_t* schemaSize);

  /**
   * @brief Create a device based on JSON input.
   * @param deviceJson Pointer to the JSON input for the device.
   * @param aggregator Pointer to the aggregator.
   * @return Pointer to the created device.
   */
  BaseDeviceInterface* createDevice(JsonObject deviceJson, esp_matter::endpoint_t* aggregator);

  // Static functions to create specific device types
  static BaseDeviceInterface* createLight(JsonObject deviceJson, esp_matter::endpoint_t* aggregator);
  static BaseDeviceInterface* createFan(JsonObject deviceJson, esp_matter::endpoint_t* aggregator);
  static BaseDeviceInterface* createPlugin(JsonObject deviceJson, esp_matter::endpoint_t* aggregator);
  static BaseDeviceInterface* createButton(JsonObject deviceJson, esp_matter::endpoint_t* aggregator);
  static BaseDeviceInterface* createWindow(JsonObject deviceJson, esp_matter::endpoint_t* aggregator);

 private:
  static uint8_t getButtonPin(uint8_t pin);
  static uint8_t getRelayPin(uint8_t pin);
};
