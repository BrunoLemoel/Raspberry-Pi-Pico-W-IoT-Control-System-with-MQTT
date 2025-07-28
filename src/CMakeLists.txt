# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# CMakeLists.txt

cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(wifimqtt C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w)
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
-Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
-Wno-unused-function # we have some for the docs that aren't called
-Wno-maybe-uninitialized
)

if (PICO_CYW43_SUPPORTED) # set by BOARD=pico-w
    set(WIFI_SSID "ALEXSANDRO" CACHE INTERNAL "WiFi SSID")
    set(WIFI_PASSWORD "g1c2f3ab" CACHE INTERNAL "WiFi password")
    if (NOT TARGET pico_cyw43_arch)
        message("Skipping build as support is not available")
    else()
        set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID")
        set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password")

        if ("${WIFI_SSID}" STREQUAL "")
            message("Skipping build as WIFI_SSID is not defined")
        elseif ("${WIFI_PASSWORD}" STREQUAL "")
            message("Skipping build as WIFI_PASSWORD is not defined")
        else()
            add_executable(wifimqtt
            main.c
            )

            # enable usb output, disable uart output
            pico_enable_stdio_usb(wifimqtt 0)
            pico_enable_stdio_usb(wifimqtt 1)

            target_compile_definitions(wifimqtt PRIVATE
            WIFI_SSID=\"${WIFI_SSID}\"
            WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
            TEST_TCP_SERVER_IP=\"${TEST_TCP_SERVER_IP}\"
            NO_SYS=1
            )

            target_include_directories(wifimqtt PRIVATE
                ${CMAKE_CURRENT_LIST_DIR}
                ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
            )
            

            target_link_libraries(wifimqtt
                pico_cyw43_arch_lwip_poll
                pico_stdlib
                pico_lwip_mbedtls
                pico_mbedtls
                pico_lwip_mqtt
            )

            pico_add_extra_outputs(wifimqtt)
        endif()
    endif()
endif()

