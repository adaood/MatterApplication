#pragma once

#include <esp_err.h>

class StorageManagerInterface {
 public:
  virtual ~StorageManagerInterface() = default;
  virtual esp_err_t eraseStorage() = 0;
  virtual esp_err_t eraseDevice() = 0;
  virtual bool checkProgramMode() = 0;
  virtual esp_err_t setProgramMode(bool programMode) = 0;
  virtual esp_err_t getAccessoryDB(char *response, size_t response_size) = 0;
  virtual esp_err_t setAccessoryDB(const char *newAccessoryDB) = 0;
};