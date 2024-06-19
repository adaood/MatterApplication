#include "Creator.hpp"

#include <ArduinoJson.h>
#include <esp_matter.h>

#include <BlindAccessory.hpp>
#include <ButtonDevice.hpp>
#include <ButtonModule.hpp>
#include <FanAccessory.hpp>
#include <FanDevice.hpp>
#include <LightAccessory.hpp>
#include <LightDevice.hpp>
#include <PluginAccessory.hpp>
#include <PluginDevice.hpp>
#include <RelayModule.hpp>
#include <StatelessButtonAccessory.hpp>
#include <WindowDevice.hpp>

namespace Creator {
esp_err_t Light(JsonObject accessory, esp_matter::endpoint_t *aggregator) {
  const char *name = accessory["name"].as<const char *>();
  uint32_t aid = accessory["aidMeta"].as<uint32_t>();
  uint8_t lightPin = accessory["lightPin"].as<uint8_t>();
  uint8_t buttonPin = accessory["buttonPin"].as<uint8_t>();

  ButtonModule *button = new ButtonModule(GetButtonPin(buttonPin));
  RelayModule *relay = new RelayModule(GetRelayPin(lightPin));

  LightAccessory *lightAccessory = new LightAccessory(relay, button);

  LightDevice *lightDevice = new LightDevice(name, lightAccessory, aggregator);

  return ESP_OK;
}

esp_err_t Fan(JsonObject accessory, esp_matter::endpoint_t *aggregator) {
  const char *name = accessory["name"].as<const char *>();
  uint32_t aid = accessory["aidMeta"].as<uint32_t>();
  uint8_t fanPin = accessory["fanPin"].as<uint8_t>();
  uint8_t buttonPin = accessory["buttonPin"].as<uint8_t>();

  ButtonModule *button = new ButtonModule(GetButtonPin(buttonPin));
  RelayModule *relay = new RelayModule(GetRelayPin(fanPin));

  FanAccessory *fanAccessory = new FanAccessory(relay, button);

  FanDevice *fanDevice = new FanDevice(name, fanAccessory, aggregator);
  return ESP_OK;
}

esp_err_t Button(JsonObject accessory, esp_matter::endpoint_t *aggregator) {
  const char *name = accessory["name"].as<const char *>();
  uint32_t aid = accessory["aidMeta"].as<uint32_t>();
  uint8_t buttonPin = accessory["buttonPin"].as<uint8_t>();

  ButtonModule *button = new ButtonModule(GetButtonPin(buttonPin));

  StatelessButtonAccessory *buttonAccessory = new StatelessButtonAccessory(button);

  ButtonDevice *buttonDevice = new ButtonDevice(name, buttonAccessory, aggregator);

  return ESP_OK;
}

esp_err_t Plugin(JsonObject accessory, esp_matter::endpoint_t *aggregator) {
  const char *name = accessory["name"].as<const char *>();
  uint32_t aid = accessory["aidMeta"].as<uint32_t>();
  uint8_t outletPin = accessory["outletPin"].as<uint8_t>();
  uint8_t buttonPin = accessory["buttonPin"].as<uint8_t>();

  ButtonModule *button = new ButtonModule(GetButtonPin(buttonPin));
  RelayModule *relay = new RelayModule(GetRelayPin(outletPin));

  PluginAccessory *pluginAccessory = new PluginAccessory(relay, button);

  PlugInDevice *pluginDevice = new PlugInDevice(name, pluginAccessory, aggregator);

  return ESP_OK;
}

esp_err_t Window(JsonObject accessory, esp_matter::endpoint_t *aggregator) {
  const char *name = accessory["name"].as<const char *>();
  uint32_t aid = accessory["aidMeta"].as<uint32_t>();
  uint8_t motorUpPin = accessory["motorUpPin"].as<uint8_t>();
  uint8_t motorDownPin = accessory["motorDownPin"].as<uint8_t>();
  uint8_t buttonUpPin = accessory["buttonUpPin"].as<uint8_t>();
  uint8_t buttonDownPin = accessory["buttonDownPin"].as<uint8_t>();
  uint16_t timeToClose = accessory["timeToClose"].as<uint16_t>();
  uint16_t timeToOpen = accessory["timeToOpen"].as<uint16_t>();

  ButtonModule *buttonUp = new ButtonModule(GetButtonPin(buttonUpPin));
  ButtonModule *buttonDown = new ButtonModule(GetButtonPin(buttonDownPin));
  RelayModule *motorUp = new RelayModule(GetRelayPin(motorUpPin));
  RelayModule *motorDown = new RelayModule(GetRelayPin(motorDownPin));

  BlindAccessory *windowAccessory =
      new BlindAccessory(motorUp, motorDown, buttonUp, buttonDown, timeToOpen, timeToClose);

  WindowDevice *windowDevice = new WindowDevice(name, windowAccessory, aggregator);

  return ESP_OK;
}

uint8_t GetButtonPin(uint8_t pin) {
  // uint8_t buttonPins[] = {14, 27, 26, 25, 33, 32, 35, 34};
  uint8_t buttonPins[] = {34, 35, 32, 33, 25, 26, 27, 14};
  return buttonPins[(pin - 1) % 8];
}

uint8_t GetRelayPin(uint8_t pin) {
  uint8_t relayPins[] = {23, 22, 21, 19, 18, 17, 16, 4};
  return relayPins[(pin - 1) % 8];
}
}  // namespace Creator