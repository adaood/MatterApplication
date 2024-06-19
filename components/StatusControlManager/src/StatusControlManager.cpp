#include "StatusControlManager.hpp"

#include <esp_system.h>

StatusControlManager::StatusControlManager(ButtonModuleInterface *buttonModule,
                                           RelayModuleInterface *relayModule,
                                           StorageManagerInterface *storageManager, STATUS_MODE mode)
    : buttonModule(buttonModule),
      relayModule(relayModule),
      storageManager(storageManager),
      statusMode(mode),
      blinkTaskHandle(nullptr) {
  // Set the button module to use the buttonPressed method of this class.
  buttonModule->onSinglePress(buttonSingleFunction, this);
  buttonModule->onLongPress(buttonLongFunction, this);

  setStatusMode(mode);
}

StatusControlManager::~StatusControlManager() {
  // Delete the blinking task.
  if (blinkTaskHandle != nullptr) vTaskDelete(blinkTaskHandle);
}

STATUS_MODE StatusControlManager::getStatusMode() { return statusMode; }

void StatusControlManager::setStatusMode(STATUS_MODE mode) {
  // Set the status mode.
  statusMode = mode;

  // Delete the blinking task.
  if (blinkTaskHandle != nullptr) {
    vTaskDelete(blinkTaskHandle);
    blinkTaskHandle = nullptr;
  }

  startLedTask();
}

void StatusControlManager::startLedTask() {
  relayModule->setPower(false);
  switch (statusMode) {
    case STATUS_MODE::RUNNING:
      relayModule->setPower(true);
      break;
    case STATUS_MODE::PROGRAMMING:
      xTaskCreate(
          [](void *pParameter) {
            StatusControlManager *self = static_cast<StatusControlManager *>(pParameter);
            self->ledBlinkingTask(self->programModeTimings[0], self->programModeTimings[1],
                                  self->programModeTimings[2], self->programModeTimings[3]);
          },
          "blinkTask", 3000, this, 1, &blinkTaskHandle);
      break;
    case STATUS_MODE::TRY_CONNECTING:
      xTaskCreate(
          [](void *pParameter) {
            StatusControlManager *self = static_cast<StatusControlManager *>(pParameter);
            self->ledBlinkingTask(self->tryConnectTimings[0], self->tryConnectTimings[1],
                                  self->tryConnectTimings[2], self->tryConnectTimings[3]);
          },
          "blinkTask", 3000, this, 1, &blinkTaskHandle);
      break;
    case STATUS_MODE::SEARCH_WIFI:
      xTaskCreate(
          [](void *pParameter) {
            StatusControlManager *self = static_cast<StatusControlManager *>(pParameter);
            self->ledBlinkingTask(self->searchWifiTimings[0], self->searchWifiTimings[1],
                                  self->searchWifiTimings[2], self->searchWifiTimings[3]);
          },
          "blinkTask", 3000, this, 1, &blinkTaskHandle);
      break;
  }
}

void StatusControlManager::ledBlinkingTask(uint16_t firstON, uint16_t firstOFF, uint16_t secondON,
                                           uint16_t secondOFF) {
  while (true) {
    relayModule->setPower(true);
    vTaskDelay(firstON / portTICK_PERIOD_MS);
    relayModule->setPower(false);
    vTaskDelay(firstOFF / portTICK_PERIOD_MS);
    relayModule->setPower(true);
    vTaskDelay(secondON / portTICK_PERIOD_MS);
    relayModule->setPower(false);
    vTaskDelay(secondOFF / portTICK_PERIOD_MS);
  }
}

void StatusControlManager::buttonSingleFunction(void *self) {
  StatusControlManager *statusControlManager = static_cast<StatusControlManager *>(self);

  statusControlManager->storageManager->setProgramMode(false);
  esp_restart();
}

void StatusControlManager::buttonLongFunction(void *self) {
  StatusControlManager *statusControlManager = static_cast<StatusControlManager *>(self);

  statusControlManager->storageManager->setProgramMode(true);

  esp_restart();
}