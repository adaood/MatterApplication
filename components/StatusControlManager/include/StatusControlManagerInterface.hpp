#pragma once

/**
 * @brief Enumeration of the different device status modes.
 */
enum class DeviceStatusMode {
  WaitingForPairing,     ///< Device is waiting for pairing
  WaitingForConnection,  ///< Device is waiting for connection
  RunningAsExpected,     ///< Device is running as expected
  InProgramMode,         ///< Device is in program mode
};

/**
 * @brief Interface for the Status Control Manager.
 */
class StatusControlManagerInterface {
 public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~StatusControlManagerInterface() = default;

  /**
   * @brief Get the current status mode of the device.
   * @return The current status mode.
   */
  virtual DeviceStatusMode getCurrentStatusMode() const = 0;

  /**
   * @brief Update the status mode of the device.
   * @param mode The new status mode to set.
   */
  virtual void updateStatusMode(DeviceStatusMode mode) = 0;

  /**
   * @brief Start the status control manager.
   */
  virtual void start() = 0;
};
