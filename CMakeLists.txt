cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts -fcommon -fno-rtti -fno-exceptions")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PICO_DEOPTIMIZED_DEBUG 1)

# path to PICO utilities (if needed)
if (NOT DEFINED ENV{PIOASM_DIR})
        message("PIOASM: set PIOASM_DIR env variable if you use PIO")
else()
        message("PIOASM: PIOASM_DIR env variable is set to '$ENV{PIOASM_DIR}'")
        set(pioasm_DIR $ENV{PIOASM_DIR})
endif()
if (NOT DEFINED ENV{PICOTOOL_DIR})
        message("PICOTOOL: set PICOTOOL_DIR env variable if you create uf2 output with pico_add_extra_outputs()")
else()
        message("PICOTOOL: PICOTOOL_DIR env variable is set to '$ENV{PICOTOOL_DIR}'")
        set(picotool_DIR $ENV{PICOTOOL_DIR})
endif()

if (DEFINED ENV{PICO_PLATFORM})
        message("PICO_PLATFORM: PICO_PLATFORM env variable is set to '$ENV{PICO_PLATFORM}'")
        set(PICO_PLATFORM $ENV{PICO_PLATFORM} CACHE STRING "Pico Platform")
else()
        message("PICO_PLATFORM: PICO_PLATFORM env variable is not set. Using default")
endif()

if (DEFINED ENV{PICO_BOARD})
        message("PICO_BOARD: PICO_BOARD env variable is set to '$ENV{PICO_BOARD}'")
        set(PICO_BOARD $ENV{PICO_BOARD} CACHE STRING "Board type")
else()
        message("PICO_BOARD: PICO_BOARD env variable is not set. Using default")
endif()

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)


# Pull in FreeRTOS
include(FreeRTOS_Kernel_import.cmake)


project(pst_rtos C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

pico_add_subdirectory(pico_scpi_usbtmc_lablib)


add_executable(${CMAKE_PROJECT_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/source/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/usb/usb_descriptors.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/scpi/scpi-def.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/scpi/registers/registers.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/gpio/gpio_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/adc/adc_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/i2c/i2c_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/pwm/pwm_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/freertos/freertos_hook.c
        )

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/source
        ${CMAKE_CURRENT_LIST_DIR}/source/usb
        ${CMAKE_CURRENT_LIST_DIR}/source/scpi
        ${CMAKE_CURRENT_LIST_DIR}/source/gpio
        ${CMAKE_CURRENT_LIST_DIR}/source/adc
        ${CMAKE_CURRENT_LIST_DIR}/source/i2c
        ${CMAKE_CURRENT_LIST_DIR}/source/pwm
)

pico_set_program_name(${CMAKE_PROJECT_NAME} ${CMAKE_PROJECT_NAME})
pico_set_program_version(${CMAKE_PROJECT_NAME} "0.1")

pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)
pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 0)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        PICO_STDIO_STACK_BUFFER_SIZE=64 # use a small printf on stack buffer
        PRIVATE SCPI_USER_CONFIG=1
)
target_link_libraries(${CMAKE_PROJECT_NAME} pico_stdlib 
        FreeRTOS-Kernel FreeRTOS-Kernel-Heap4 
        tinyusb_device tinyusb_board hardware_gpio hardware_adc 
        hardware_pwm hardware_i2c pico_unique_id
        pico_scpi_usbtmc_lablib
        )

# TODO is this script multi-core safe?
# pico_set_linker_script(${CMAKE_PROJECT_NAME} ${CMAKE_SOURCE_DIR}/memmap_custom.ld)

# pico_add_extra_outputs(${CMAKE_PROJECT_NAME})


