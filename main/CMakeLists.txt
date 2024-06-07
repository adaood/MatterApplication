cmake_minimum_required(VERSION 3.5)

file(GLOB SRC_FILES "src/*.cpp")

idf_component_register(SRCS "${SRC_FILES}"
                       INCLUDE_DIRS "include"
                       REQUIRES)

# matter
set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
# end matter