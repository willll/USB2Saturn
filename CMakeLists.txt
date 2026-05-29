cmake_minimum_required(VERSION 3.13)



if(NOT DEFINED PICO_BOARD)
    set(PICO_BOARD pico2)
endif()

include(pico_sdk_import.cmake)

project(USB2Saturn C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

add_executable(USB2Saturn
    src/main.c
    src/usb_host.c
    src/saturn_out.c
)
set_target_properties(USB2Saturn PROPERTIES SUFFIX ".elf")

# Add the directory containing tusb_config.h to the include paths
target_include_directories(USB2Saturn PRIVATE
    src
)

# Pull in our pico_stdlib and the TinyUSB host stack
target_link_libraries(USB2Saturn
    pico_stdlib
    tinyusb_host
    tinyusb_board
)

# We want USB host support, and no stdio over USB since USB is used for host
pico_enable_stdio_usb(USB2Saturn 0)
pico_enable_stdio_uart(USB2Saturn 1)

# Enable PIO support
target_link_libraries(USB2Saturn hardware_pio)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(USB2Saturn)
