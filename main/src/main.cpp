#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>

#include "AccessPoint.hpp"
#include "ButtonModule.hpp"
#include "EndpointManager.hpp"
#include "RelayModule.hpp"
#include "StatusControlManager.hpp"
#include "StorageManager.hpp"

static const char *TAG = "main";

extern "C" void app_main() {
  ESP_ERROR_CHECK(nvs_flash_init());

  ESP_LOGI(TAG, "Starting Access Point Manager");

  // create an instance of the StorageManager class
  StorageManager *storageManager = new StorageManager();
  ButtonModule *buttonModule = new ButtonModule(5);
  RelayModule *relayModule = new RelayModule(2);
  StatusControlManager *statusControlManager =
      new StatusControlManager(storageManager, relayModule, buttonModule);
  AccessPoint *accessPoint;
  EndpointManager *endpointManager;

  int numAcc = 4000;
  char *jsonArray = (char *)calloc(numAcc, sizeof(char));

  bool progFlag = false;
  if (storageManager->isProgramModeEnabled(&progFlag) == ESP_OK && (progFlag == true)) {
    // create an instance of the AccessPoint class
    statusControlManager->updateStatusMode(DeviceStatusMode::InProgramMode);
    accessPoint = new AccessPoint(storageManager);
    accessPoint->startWebServer();
  } else {
    storageManager->getAccessoryJson(jsonArray, numAcc);
    if (strlen(jsonArray) <= 0) {
      statusControlManager->updateStatusMode(DeviceStatusMode::InProgramMode);
      accessPoint = new AccessPoint(storageManager);
      accessPoint->startWebServer();
    } else {
      // create an instance of the EndpointManager class
      statusControlManager->updateStatusMode(DeviceStatusMode::RunningAsExpected);
      endpointManager = new EndpointManager(true);
      endpointManager->createArrayOfEndpoints(jsonArray, strlen(jsonArray));
      endpointManager->startMatter();
    }
  }
}