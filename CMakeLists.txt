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
    src/input_mapper.c
    src/saturn_keyboard_protocol.c
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

option(USB2SATURN_ENABLE_HOST_TESTS "Build host unit tests (non-cross builds only)" ON)

if(USB2SATURN_ENABLE_HOST_TESTS AND NOT CMAKE_CROSSCOMPILING)
    include(CTest)

    add_executable(usb2saturn_input_mapper_tests
        tests/test_input_mapper.c
        src/input_mapper.c
    )

    target_include_directories(usb2saturn_input_mapper_tests PRIVATE
        src
    )

    add_test(NAME usb2saturn_input_mapper_tests COMMAND usb2saturn_input_mapper_tests)

    add_executable(usb2saturn_keyboard_protocol_tests
        tests/test_keyboard_protocol.c
        src/saturn_keyboard_protocol.c
    )

    target_include_directories(usb2saturn_keyboard_protocol_tests PRIVATE
        src
    )

    add_test(NAME usb2saturn_keyboard_protocol_tests COMMAND usb2saturn_keyboard_protocol_tests)
endif()
