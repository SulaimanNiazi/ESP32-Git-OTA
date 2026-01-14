# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html

cmake_minimum_required(VERSION 3.5)

set(hardware "ESP32WROOM32")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_definitions(HARDWARE="${hardware}")

project(Git_OTA VERSION 3.1.1)
