#pragma once

#include <esp_event.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <ButtonModuleInterface.hpp>
#include <RelayModuleInterface.hpp>

#include "StatusControlManagerInterface.hpp"
#include "StorageManagerInterface.hpp"

/**
 * @brief Manages the status of the device, including Wi-Fi events and status modes.
 */
class StatusControlManager : public StatusControlManagerInterface {
 public:
  /**
   * @brief Constructor.
   * @param storageManager Pointer to the storage manager interface.
   * @param relayModule Pointer to the relay module interface.
   * @param buttonModule Pointer to the button module interface.
   */
  StatusControlManager(StorageManagerInterface* storageManager = nullptr,
                       RelayModuleInterface* relayModule = nullptr,
                       ButtonModuleInterface* buttonModule = nullptr);

  /**
   * @brief Destructor.
   */
  ~StatusControlManager() override;

  /**
   * @brief Get the current status mode of the device.
   * @return The current status mode.
   */
  DeviceStatusMode getCurrentStatusMode() const override;

  /**
   * @brief Update the status mode of the device.
   * @param mode The new status mode to set.
   */
  void updateStatusMode(DeviceStatusMode mode) override;

  /**
   * @brief Start the status control manager.
   */
  void start() override;

 private:
  DeviceStatusMode m_currentStatusMode;       ///< Current status mode of the device
  StorageManagerInterface* m_storageManager;  ///< Pointer to the storage manager interface
  RelayModuleInterface* m_relayModule;        ///< Pointer to the relay module interface
  ButtonModuleInterface* m_buttonModule;      ///< Pointer to the button module interface
  TaskHandle_t* m_blinkTaskHandle;            ///< Handle of the blink task

  /**
   * @brief Initialize the Wi-Fi event listener.
   */
  void initWiFiEventListener();

  /**
   * @brief Set the button callbacks.
   */
  void setButtonCallbacks();

  /**
   * @brief Callback for the button SINGLE press event.
   */
  void resetartCallBack();

  /**
   * @brief Callback for the button DOUBLE press event.
   */
  void programModeCallBack();

  /**
   * @brief Callback for the button LONG press event.
   */
  void factoryResetCallBack();

  /**
   * @brief Wi-Fi event handler.
   * @param arg Pointer to the argument.
   * @param eventBase Base of the event.
   * @param eventId ID of the event.
   * @param eventData Pointer to the event data.
   */
  static void wifiEventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData);

  /**
   * @brief Task to blink the LED based on the current status mode.
   */
  void ledBlinkingTask();

  /**
   * @brief Start the LED blinking task based on the current status mode.
   */
  void startLedTask();

  // Delete the copy constructor and assignment operator
  StatusControlManager(const StatusControlManager&) = delete;
  StatusControlManager& operator=(const StatusControlManager&) = delete;
};
