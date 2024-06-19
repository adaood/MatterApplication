#pragma once

/**
 * @file StatusControlManagerInterface.hpp
 * @brief Defines the interface for the StatusControlManager class.
 */

enum class STATUS_MODE {
  RUNNING,         ///< The system is running normally.
  PROGRAMMING,     ///< The system is in programming mode.
  TRY_CONNECTING,  ///< The system is trying to establish a connection.
  SEARCH_WIFI,     ///< The system is searching for a WiFi network.
};

/**
 * @class StatusControlManagerInterface
 * @brief Interface for managing the status led and control button
 */
class StatusControlManagerInterface {
 public:
  virtual ~StatusControlManagerInterface() = default;

  /**
   * @brief Get the current status mode.
   * @return The current status mode.
   */
  virtual STATUS_MODE getStatusMode() = 0;

  /**
   * @brief Set the status mode.
   * @details This method will change the status mode and start the blinking task.
   * @param mode The new status mode to set.
   */
  virtual void setStatusMode(STATUS_MODE mode) = 0;
};
