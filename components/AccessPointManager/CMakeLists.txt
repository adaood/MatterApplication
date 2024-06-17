cmake_minimum_required(VERSION 3.5)

file(GLOB SRC_FILES "src/*.cpp")

idf_component_register(SRCS "${SRC_FILES}"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_http_server StorageManager
                       PRIV_REQUIRES spiffs esp_wifi)

set(DATA_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data/frontend")
spiffs_create_partition_image(frontend ${DATA_SRC_DIR} FLASH_IN_PROJECT)