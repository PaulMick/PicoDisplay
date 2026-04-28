# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(PicoDisplay C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(PicoDisplay
    main.c
    picodisplay.c
    display_driver.c
    display_utils.c
    ntp.c
    wifi.c
    dp_states/clock_hms_state.c
)

pico_set_program_name(PicoDisplay "PicoDisplay")
pico_set_program_version(PicoDisplay "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(PicoDisplay 0)
pico_enable_stdio_usb(PicoDisplay 1)

# Add the standard library to the build
target_link_libraries(PicoDisplay
    pico_stdlib
)

# Add the standard include files to the build
target_include_directories(PicoDisplay PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Compile PIO code
pico_generate_pio_header(PicoDisplay
    ${CMAKE_CURRENT_LIST_DIR}/hub75.pio
)

# Add any user requested libraries
pico_add_library(lwip_http_util NOFLAG)
pico_mirrored_target_link_libraries(lwip_http_util INTERFACE
    pico_lwip_http
    pico_lwip_mbedtls
    pico_mbedtls
)
target_link_libraries(PicoDisplay
    pico_cyw43_arch_lwip_threadsafe_background
    lwip_http_util
    hardware_pio
    pico_multicore
)
set_source_files_properties(
    ${PICO_LWIP_PATH}/src/apps/altcp_tls/altcp_tls_mbedtls.c
    PROPERTIES
    COMPILE_OPTIONS "-Wno-unused-result"
)

pico_add_extra_outputs(PicoDisplay)

