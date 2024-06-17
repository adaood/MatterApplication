#pragma once

#include <esp_err.h>

#include "StorageManagerInterface.hpp"

class StorageManager : public StorageManagerInterface {
  // delete the copy constructor and the assignment operator
  StorageManager(const StorageManager &) = delete;
  StorageManager &operator=(const StorageManager &) = delete;

 public:
  // should call nvs_flash_init() before calling this constructor
  StorageManager();
  ~StorageManager() override;
  esp_err_t eraseStorage() override;
  esp_err_t eraseDevice() override;
  bool checkProgramMode() override;
  esp_err_t setProgramMode(bool programMode) override;
  esp_err_t getAccessoryDB(char *response, size_t response_size) override;
  esp_err_t setAccessoryDB(const char *newAccessoryDB) override;
};