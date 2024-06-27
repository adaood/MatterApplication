#include "StatusControlManager.hpp"

#include <esp_log.h>

#include <ButtonModule.hpp>
#include <RelayModule.hpp>

static const char* TAG = "StatusControlManager";

StatusControlManager::StatusControlManager(StorageManagerInterface* storageManager,
                                           RelayModuleInterface* relayModule,
                                           ButtonModuleInterface* buttonModule)
    : m_storageManager(storageManager),
      m_relayModule(relayModule),
      m_buttonModule(buttonModule),
      m_currentStatusMode(DeviceStatusMode::WaitingForPairing),
      m_blinkTaskHandle(nullptr),
      internalRelayModule(relayModule == nullptr),
      internalButtonModule(buttonModule == nullptr) {
  if (m_relayModule == nullptr) {
    ESP_LOGI(TAG, "Creating default relay module");
    m_relayModule = new RelayModule(CONFIG_S_C_M_STATUS_LED_PIN, 1, 1);
  }
  if (m_buttonModule == nullptr) {
    ESP_LOGI(TAG, "Creating default button module");
    m_buttonModule =
        new ButtonModule(CONFIG_S_C_M_CONTROL_BUTTON_PIN, 1, CONFIG_S_C_M_CONTROL_BUTTON_DEBOUNCE_DELAY,
                         CONFIG_S_C_M_CONTROL_BUTTON_LONG_PRESS_DELAY);
  }
}

StatusControlManager::~StatusControlManager() {
  if (m_blinkTaskHandle != nullptr) {
    vTaskDelete(*m_blinkTaskHandle);
    m_blinkTaskHandle = nullptr;
  }
  if (m_relayModule != nullptr && internalRelayModule) {
    delete m_relayModule;
    m_relayModule = nullptr;
  }
  if (m_buttonModule != nullptr && internalButtonModule) {
    delete m_buttonModule;
    m_buttonModule = nullptr;
  }
}

DeviceStatusMode StatusControlManager::getCurrentStatusMode() const { return m_currentStatusMode; }

void StatusControlManager::updateStatusMode(DeviceStatusMode mode) {
  m_currentStatusMode = mode;
  ESP_LOGI(TAG, "Status mode changed to:");
  startLedTask();
}

void StatusControlManager::start() {
  initWiFiEventListener();
  updateStatusMode(DeviceStatusMode::WaitingForPairing);
  setButtonCallbacks();
}

void StatusControlManager::initWiFiEventListener() {
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, this));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, this));
}

void StatusControlManager::wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId,
                                            void* eventData) {
  StatusControlManager* manager = static_cast<StatusControlManager*>(arg);

  if (eventBase == WIFI_EVENT) {
    if (eventId == WIFI_EVENT_STA_DISCONNECTED || eventId == WIFI_EVENT_STA_START) {
      manager->updateStatusMode(DeviceStatusMode::WaitingForConnection);
    } else if (eventId == WIFI_EVENT_AP_START) {
      manager->updateStatusMode(DeviceStatusMode::InProgramMode);
    }
  } else if (eventBase == IP_EVENT) {
    if (eventId == IP_EVENT_STA_GOT_IP) {
      manager->updateStatusMode(DeviceStatusMode::RunningAsExpected);
    }
  }
}

void StatusControlManager::startLedTask() {
  m_relayModule->setPower(false);
  if (m_blinkTaskHandle != nullptr) {
    vTaskDelete(*m_blinkTaskHandle);
    m_blinkTaskHandle = nullptr;
  }

  xTaskCreate(
      [](void* arg) {
        StatusControlManager* manager = static_cast<StatusControlManager*>(arg);
        manager->ledBlinkingTask();
      },
      "ledBlinkingTask", CONFIG_S_C_M_BLINKING_TASK_STACK_SIZE, this, CONFIG_S_C_M_BLINKING_TASK_PRIORITY,
      m_blinkTaskHandle);
}

void StatusControlManager::ledBlinkingTask() {
  uint16_t delay1 = 0;
  uint16_t delay2 = 0;
  uint16_t delay3 = 0;
  uint16_t delay4 = 0;
  switch (m_currentStatusMode) {
    case DeviceStatusMode::WaitingForPairing:
      delay1 = 250;
      delay2 = 250;
      delay3 = 250;
      delay4 = 250;
      break;
    case DeviceStatusMode::WaitingForConnection:
      delay1 = 250;
      delay2 = 1000;
      delay3 = 250;
      delay4 = 1000;
      break;
    case DeviceStatusMode::InProgramMode:
      delay1 = 250;
      delay2 = 100;
      delay3 = 250;
      delay4 = 1000;
      break;
    default:
      m_relayModule->setPower(true);
      m_blinkTaskHandle = nullptr;
      vTaskDelete(*m_blinkTaskHandle);
      break;
  }

  while (true) {
    m_relayModule->setPower(true);
    vTaskDelay(delay1 / portTICK_PERIOD_MS);
    m_relayModule->setPower(false);
    vTaskDelay(delay2 / portTICK_PERIOD_MS);
    m_relayModule->setPower(true);
    vTaskDelay(delay3 / portTICK_PERIOD_MS);
    m_relayModule->setPower(false);
    vTaskDelay(delay4 / portTICK_PERIOD_MS);
  }
}

void StatusControlManager::setButtonCallbacks() {
  m_buttonModule->setSinglePressCallback([](void* arg) {
    StatusControlManager* manager = static_cast<StatusControlManager*>(arg);
    manager->resetartCallBack();
  });

  m_buttonModule->setDoublePressCallback([](void* arg) {
    StatusControlManager* manager = static_cast<StatusControlManager*>(arg);
    manager->programModeCallBack();
  });

  m_buttonModule->setLongPressCallback([](void* arg) {
    StatusControlManager* manager = static_cast<StatusControlManager*>(arg);
    manager->factoryResetCallBack();
  });
}

void StatusControlManager::resetartCallBack() {
  if (m_storageManager != nullptr) {
    m_storageManager->setProgramMode(false);
  }
  esp_restart();
}

void StatusControlManager::programModeCallBack() {
  if (m_storageManager != nullptr) {
    m_storageManager->setProgramMode(true);
  }
  esp_restart();
}

void StatusControlManager::factoryResetCallBack() {
  if (m_storageManager != nullptr) {
    m_storageManager->eraseAllData();
  }
  esp_restart();
}