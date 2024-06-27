cmake_minimum_required(VERSION 3.5)

file(GLOB SRC_FILES "src/*.cpp")

idf_component_register(SRCS "${SRC_FILES}"
                       INCLUDE_DIRS "include"
                       REQUIRES StorageManager esp_wifi esp_event
                       PRIV_REQUIRES)
