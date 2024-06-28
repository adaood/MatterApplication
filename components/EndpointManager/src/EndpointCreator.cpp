#include "EndpointCreator.hpp"

#include <esp_log.h>

#include <BlindAccessory.hpp>
#include <ButtonModule.hpp>
#include <FanAccessory.hpp>
#include <LightAccessory.hpp>
#include <PluginAccessory.hpp>
#include <RelayModule.hpp>
#include <StatelessButtonAccessory.hpp>

static const char* TAG = "EndpointCreator";

static const char* lightJsonProprties = "[\"name\", \"lightPin\", \"buttonPin\"]";
static const char* fanJsonProprties = "[\"name\", \"fanPin\", \"buttonPin\"]";
static const char* pluginJsonProprties = "[\"name\", \"pluginPin\", \"buttonPin\"]";
static const char* buttonJsonProprties = "[\"name\", \"buttonPin\"]";
static const char* windowJsonProprties =
    "[\"name\", \"motorUpPin\", \"motorDownPin\", \"buttonUpPin\", \"buttonDownPin\", \"timeToOpen\", "
    "\"timeToClose\"]";

static const DeviceType m_deviceTypes[] = {{"LIGHT", DeviceCreator::createLight},
                                           {"FAN", DeviceCreator::createFan},
                                           {"PLUGIN", DeviceCreator::createPlugin},
                                           {"BUTTON", DeviceCreator::createButton},
                                           {"WINDOW", DeviceCreator::createWindow}};

void DeviceCreator::getJsonSchemaForAllDevices(char* jsonSchema, size_t schemaSize) {
  snprintf(jsonSchema, schemaSize, "{\"LIGHT\":%s,\"FAN\":%s,\"PLUGIN\":%s,\"BUTTON\":%s,\"WINDOW\":%s}",
           lightJsonProprties, fanJsonProprties, pluginJsonProprties, buttonJsonProprties,
           windowJsonProprties);
};

void DeviceCreator::getJsonSchemaSizeForAllDevices(size_t* schemaSize) {
  *schemaSize = 0;
  *schemaSize += strlen(lightJsonProprties);
  *schemaSize += strlen(fanJsonProprties);
  *schemaSize += strlen(pluginJsonProprties);
  *schemaSize += strlen(buttonJsonProprties);
  *schemaSize += strlen(windowJsonProprties);
  *schemaSize += 5;  // for the commas and the brackets
};

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

BaseDeviceInterface* DeviceCreator::createLight(JsonObject deviceJson, esp_matter::endpoint_t* aggregator) {
  // Parse the JSON properties array
  StaticJsonDocument<200> doc;
  deserializeJson(doc, lightJsonProprties);
  JsonArray properties = doc.as<JsonArray>();

  // Extract the required properties from the deviceJson
  const char* name = deviceJson[(properties[0].as<JsonString>())].as<const char*>();
  uint8_t lightPin = deviceJson[properties[1].as<JsonString>()].as<uint8_t>();
  uint8_t buttonPin = deviceJson[properties[2].as<JsonString>()].as<uint8_t>();

  // Extract lightJsonProprties and initialize the light device
  ButtonModule* button = new ButtonModule(getButtonPin(buttonPin));
  RelayModule* relay = new RelayModule(getRelayPin(lightPin));

  LightAccessory* lightAccessory = new LightAccessory(relay, button);
  LightDevice* lightDevice = new LightDevice((char*)name, lightAccessory, aggregator);
  return lightDevice;
}

BaseDeviceInterface* DeviceCreator::createFan(JsonObject deviceJson, esp_matter::endpoint_t* aggregator) {
  // Parse the JSON properties array
  StaticJsonDocument<200> doc;
  deserializeJson(doc, fanJsonProprties);
  JsonArray properties = doc.as<JsonArray>();

  const char* name = deviceJson[(properties[0].as<JsonString>())].as<const char*>();
  uint8_t fanPin = deviceJson[properties[1].as<JsonString>()].as<uint8_t>();
  uint8_t buttonPin = deviceJson[properties[2].as<JsonString>()].as<uint8_t>();

  // Extract fanJsonProprties and initialize the fan device
  ButtonModule* button = new ButtonModule(getButtonPin(buttonPin));
  RelayModule* relay = new RelayModule(getRelayPin(fanPin));

  FanAccessory* fanAccessory = new FanAccessory(relay, button);
  FanDevice* fanDevice = new FanDevice((char*)name, fanAccessory, aggregator);
  return fanDevice;
}

BaseDeviceInterface* DeviceCreator::createPlugin(JsonObject deviceJson, esp_matter::endpoint_t* aggregator) {
  // Parse the JSON properties array
  StaticJsonDocument<200> doc;
  deserializeJson(doc, pluginJsonProprties);
  JsonArray properties = doc.as<JsonArray>();

  const char* name = deviceJson[(properties[0].as<JsonString>())].as<const char*>();
  uint8_t pluginPin = deviceJson[properties[1].as<JsonString>()].as<uint8_t>();
  uint8_t buttonPin = deviceJson[properties[2].as<JsonString>()].as<uint8_t>();

  // Extract pluginJsonProprties and initialize the plugin device
  ButtonModule* button = new ButtonModule(getButtonPin(buttonPin));
  RelayModule* relay = new RelayModule(getRelayPin(pluginPin));

  PluginAccessory* pluginAccessory = new PluginAccessory(relay, button);
  PluginDevice* pluginDevice = new PluginDevice((char*)name, pluginAccessory, aggregator);
  return pluginDevice;
}

BaseDeviceInterface* DeviceCreator::createButton(JsonObject deviceJson, esp_matter::endpoint_t* aggregator) {
  // Parse the JSON properties array
  StaticJsonDocument<200> doc;
  deserializeJson(doc, buttonJsonProprties);
  JsonArray properties = doc.as<JsonArray>();

  const char* name = deviceJson[(properties[0].as<JsonString>())].as<const char*>();
  uint8_t buttonPin = deviceJson[properties[1].as<JsonString>()].as<uint8_t>();

  // Extract buttonJsonProprties and initialize the button device
  ButtonModule* button = new ButtonModule(getButtonPin(buttonPin));
  StatelessButtonAccessory* buttonAccessory = new StatelessButtonAccessory(button);
  ButtonDevice* buttonDevice = new ButtonDevice((char*)name, buttonAccessory, aggregator);
  return buttonDevice;
}

BaseDeviceInterface* DeviceCreator::createWindow(JsonObject deviceJson, esp_matter::endpoint_t* aggregator) {
  // Parse the JSON properties array
  StaticJsonDocument<200> doc;
  deserializeJson(doc, windowJsonProprties);
  JsonArray properties = doc.as<JsonArray>();

  const char* name = deviceJson[(properties[0].as<JsonString>())].as<const char*>();
  uint8_t motorUpPin = deviceJson[properties[1].as<JsonString>()].as<uint8_t>();
  uint8_t motorDownPin = deviceJson[properties[2].as<JsonString>()].as<uint8_t>();
  uint8_t buttonUpPin = deviceJson[properties[3].as<JsonString>()].as<uint8_t>();
  uint8_t buttonDownPin = deviceJson[properties[4].as<JsonString>()].as<uint8_t>();
  uint32_t timeToOpen = deviceJson[properties[5].as<JsonString>()].as<uint32_t>();
  uint32_t timeToClose = deviceJson[properties[6].as<JsonString>()].as<uint32_t>();

  // Extract windowJsonProprties and initialize the window device
  ButtonModule* buttonUp = new ButtonModule(getButtonPin(buttonUpPin));
  ButtonModule* buttonDown = new ButtonModule(getButtonPin(buttonDownPin));
  RelayModule* motorUp = new RelayModule(getRelayPin(motorUpPin));
  RelayModule* motorDown = new RelayModule(getRelayPin(motorDownPin));

  BlindAccessory* blindAccessory =
      new BlindAccessory(motorUp, motorDown, buttonUp, buttonDown, timeToOpen, timeToClose);
  WindowDevice* windowDevice = new WindowDevice((char*)name, blindAccessory, aggregator);
  return windowDevice;
}

uint8_t DeviceCreator::getButtonPin(uint8_t pin) {
  uint8_t buttonPins[] = {34, 35, 32, 33, 25, 26, 27, 14};
  return buttonPins[(pin - 1) % 8];
}

uint8_t DeviceCreator::getRelayPin(uint8_t pin) {
  uint8_t relayPins[] = {23, 22, 21, 19, 18, 17, 16, 4};
  return relayPins[(pin - 1) % 8];
}
