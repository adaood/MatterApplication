#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ButtonModuleInterface.hpp>
#include <RelayModuleInterface.hpp>
#include <StorageManagerInterface.hpp>

#include "StatusControlManagerInterface.hpp"

/**
 * @file StatusControlManager.hpp
 * @brief Manages the status led and control button.
 */

/**
 * @class StatusControlManager
 */
class StatusControlManager : public StatusControlManagerInterface {
 public:
  /**
   * @brief Constructor for the StatusControlManager class.
   * @param buttonModule The button module to use.
   * @param relayModule The relay module to use.
   */
  StatusControlManager(ButtonModuleInterface *buttonModule, RelayModuleInterface *relayModule,
                       StorageManagerInterface *storageManager, STATUS_MODE mode = STATUS_MODE::RUNNING);

  /**
   * @brief Destructor for the StatusControlManager class.
   */
  ~StatusControlManager();

  /**
   * @brief Get the current status mode.
   * @return The current status mode.
   */
  STATUS_MODE getStatusMode() override;

  /**
   * @brief Set the status mode.
   * @details This method will change the status mode and start the blinking task.
   * @param mode The new status mode to set.
   */
  void setStatusMode(STATUS_MODE mode) override;

 private:
  ButtonModuleInterface *buttonModule;
  RelayModuleInterface *relayModule;
  StorageManagerInterface *storageManager;
  STATUS_MODE statusMode;
  TaskHandle_t blinkTaskHandle;
  uint16_t programModeTimings[4] = {2000, 2000, 2000, 2000};  ///< Durations for program mode LED blinking.
  uint16_t tryConnectTimings[4] = {150, 150, 150, 1000};      ///< Durations for try connecting LED blinking.
  uint16_t searchWifiTimings[4] = {250, 250, 250, 250};       ///< Durations for search WiFi LED blinking.

  static void buttonSingleFunction(void *self);
  static void buttonLongFunction(void *self);

  /**
   * @brief Task for LED blinking based on status mode.
   * @param firstON Duration of the first ON state.
   * @param firstOFF Duration of the first OFF state.
   * @param secondON Duration of the second ON state.
   * @param secondOFF Duration of the second OFF state.
   */
  void ledBlinkingTask(uint16_t firstON, uint16_t firstOFF, uint16_t secondON, uint16_t secondOFF);

  /**
   * @brief Start the LED blinking task based on the current status mode.
   */
  void startLedTask();
};