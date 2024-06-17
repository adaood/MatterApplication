cmake_minimum_required(VERSION 3.5)

file(GLOB SRC_FILES "src/*.cpp")

idf_component_register(SRCS "${SRC_FILES}"
                       INCLUDE_DIRS "include"
                       REQUIRES
                       PRIV_REQUIRES esp_https_ota esp_wifi esp_partition app_update
                       EMBED_TXTFILES server_root_cert.pem)
