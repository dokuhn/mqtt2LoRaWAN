

# Disable in-source builds to prevent source tree corruption.
if(" ${CMAKE_SOURCE_DIR}" STREQUAL " ${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "
FATAL: In-source builds are not allowed.
       You should create a separate directory for build files.
")
endif()


cmake_minimum_required(VERSION 3.12)

project(STREAM_TEST VERSION 1.0 DESCRIPTION "test for streaming of LoRa data over mqtt")

# Save libs and executables in the same place
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Output directory for applications")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -lstdc++)
add_definitions(-DCFG_DEBUG -DCFG_eu868 -DCFG_sx1276_radio -DDEBUG_LMIC -DDEBUG_HAL -DDEBUG_RADIO -DBOOST_LOG_DYN_LINK)

add_executable(mqtt2LoRaWAN main.cpp MQTTDataStreamer.cpp POHelperClasses.cpp POCmdlineHelperClasses.cpp POConfigHelperClasses.cpp customLogger.cpp ./lmic/lmic.c ./lmic/aes.c  ./lmic/radio.c ./lmic/oslmic.c ./lora_gps_hat/debug.c ./lora_gps_hat/gpio.c ./lora_gps_hat/hal.c )

target_include_directories (mqtt2LoRaWAN PUBLIC 
                            "${CMAKE_CURRENT_SOURCE_DIR}"
                            "${CMAKE_CURRENT_SOURCE_DIR}/lmic"
                            "${CMAKE_CURRENT_SOURCE_DIR}/lora_gps_hat"
                            "/opt/libraries/paho.mqtt.cpp/src"
                            "/opt/libraries/boost/")

target_link_directories(mqtt2LoRaWAN PUBLIC "/opt/libraries/boost/stage/lib")

target_link_libraries(mqtt2LoRaWAN PUBLIC wiringPi paho-mqttpp3 paho-mqtt3as boost_program_options boost_log boost_log_setup boost_system boost_thread pthread)



