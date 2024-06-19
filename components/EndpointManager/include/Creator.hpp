#pragma once

#include <ArduinoJson.h>
#include <esp_err.h>
#include <esp_matter.h>

namespace Creator {
esp_err_t Light(JsonObject accessory, esp_matter::endpoint_t *aggregator);

esp_err_t Fan(JsonObject accessory, esp_matter::endpoint_t *aggregator);

esp_err_t Button(JsonObject accessory, esp_matter::endpoint_t *aggregator);

esp_err_t Plugin(JsonObject accessory, esp_matter::endpoint_t *aggregator);

esp_err_t Window(JsonObject accessory, esp_matter::endpoint_t *aggregator);

uint8_t GetButtonPin(uint8_t pin);

uint8_t GetRelayPin(uint8_t pin);
}  // namespace Creator