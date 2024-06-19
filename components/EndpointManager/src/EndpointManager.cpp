#include "EndpointManager.hpp"

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>

#include <BaseDevice.hpp>

#include "Creator.hpp"

static const char *TAG = "EndpointManager";

EndpointManager::EndpointManager(bool isBridge) : node(nullptr), aggregator(nullptr) {
  ESP_LOGI(TAG, "EndpointManager constructor");

  /* Initialize the Matter stack */
  esp_matter::node::config_t node_config;
  node = esp_matter::node::create(&node_config, app_attribute_cb, app_identification_cb);

  if (isBridge) {
    esp_matter::endpoint::aggregator::config_t aggregator_config;
    aggregator = esp_matter::endpoint::aggregator::create(
        node, &aggregator_config, esp_matter::endpoint_flags::ENDPOINT_FLAG_NONE, nullptr);
  }
}

EndpointManager::~EndpointManager() { ESP_LOGI(TAG, "EndpointManager destructor"); }

esp_err_t EndpointManager::createArrayOfEndpoints(const char *jsonArray, size_t jsonArraySize) {
  // Create a new DynamicJsonDocument
  DynamicJsonDocument doc(CONFIG_JSON_ACCESSORIES_LENGTH);

  // Deserialize the JSON Accessories into the DynamicJsonDocument
  deserializeJson(doc, jsonArray);

  // Get the reference to the Accessories array
  JsonArray accessories = doc.as<JsonArray>();

  // Loop through the Accessories array
  for (JsonVariant v : accessories) {
    // Get the reference to the Accessory object
    JsonObject accessory = v.as<JsonObject>();

    // Get the reference to the Accessory type
    const char *type = accessory["type"].as<const char *>();

    // Create the Accessory based on the type
    if (strcmp(type, "LIGHT") == 0)
      Creator::Light(accessory, aggregator);
    else if (strcmp(type, "FAN") == 0)
      Creator::Fan(accessory, aggregator);
    else if (strcmp(type, "BUTTON") == 0)
      Creator::Button(accessory, aggregator);
    else if (strcmp(type, "PLUGIN") == 0 || strcmp(type, "OUTLET") == 0)
      Creator::Plugin(accessory, aggregator);
    else if (strcmp(type, "WINDOW") == 0 || strcmp(type, "BLIND") == 0)
      Creator::Window(accessory, aggregator);
  }

  return ESP_OK;
}

esp_err_t EndpointManager::startMatter() {
  // start the Matter stack
  esp_matter::start(app_event_cb);

  return ESP_OK;
}

esp_err_t EndpointManager::app_identification_cb(esp_matter::identification::callback_type type,
                                                 uint16_t endpoint_id, uint8_t effect_id,
                                                 uint8_t effect_variant, void *priv_data) {
  if (type == esp_matter::identification::callback_type_t::START) {
    if (priv_data != nullptr) {
      BaseDevice *device = static_cast<BaseDevice *>(priv_data);
      if (device != nullptr) {
        device->identify();
      }
    }
  }
  return ESP_OK;
}

esp_err_t EndpointManager::app_attribute_cb(esp_matter::attribute::callback_type type, uint16_t endpoint_id,
                                            uint32_t cluster_id, uint32_t attribute_id,
                                            esp_matter_attr_val_t *val, void *priv_data) {
  if (type == esp_matter::attribute::callback_type_t::POST_UPDATE) {
    if (priv_data != nullptr) {
      BaseDevice *device = static_cast<BaseDevice *>(priv_data);
      if (device != nullptr) {
        device->updateAccessory();
      }
    }
    return ESP_OK;
  }

  return ESP_OK;
}

void EndpointManager::app_event_cb(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg) {
  switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
      ESP_LOGW(__FILENAME__, "Interface IP Address changed");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
      ESP_LOGW(__FILENAME__, "Commissioning complete");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
      ESP_LOGW(__FILENAME__, "Commissioning failed, fail safe timer expired");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
      ESP_LOGW(__FILENAME__, "Commissioning session started");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
      ESP_LOGW(__FILENAME__, "Commissioning session stopped");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
      ESP_LOGW(__FILENAME__, "Commissioning window opened");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
      ESP_LOGW(__FILENAME__, "Commissioning window closed");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
      ESP_LOGW(__FILENAME__, "Fabric removed successfully");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        chip::CommissioningWindowManager &commissionMgr =
            chip::Server::GetInstance().GetCommissioningWindowManager();
        constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(300);
        if (!commissionMgr.IsCommissioningWindowOpen()) {
          CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(
              kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
          if (err != CHIP_NO_ERROR) {
            ESP_LOGE(__FILENAME__, "Failed to open commissioning window: %s", chip::ErrorStr(err));
          }
        }
      }
      break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
      ESP_LOGW(__FILENAME__, "Fabric will be removed");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
      ESP_LOGW(__FILENAME__, "Fabric is updated");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
      ESP_LOGW(__FILENAME__, "Fabric is committed");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      ESP_LOGW(__FILENAME__, "---------------------------------");
      break;
    default:
      break;
  }
}