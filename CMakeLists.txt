cmake_minimum_required(VERSION 3.13)

project(mspmeter VERSION 0.1.0.0
        DESCRIPTION "firmware for a MSP430i2x-based digital voltage and current meter"
        LANGUAGES CXX ASM)


add_compile_options(-Werror -Wall -Wextra -pedantic -pedantic-errors
        -Wconversion -Wsign-conversion -Wreorder -Wswitch)


if (NOT (${CMAKE_SYSTEM_PROCESSOR} STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR}))
    # build firmware only when compiling for target

    set(MMCU "msp430i2041")

    add_compile_options(
            -flto -ffunction-sections -fdata-sections
            -fno-rtti
            -fconcepts
            -gstabs
            -Os
            -mmcu=${MMCU})

    add_library(meter STATIC)
    set_target_properties(meter PROPERTIES
            LINK_DEPENDS ${PROJECT_SOURCE_DIR}/src/${MMCU}.ld)
    target_compile_features(meter PUBLIC cxx_std_20)
    target_include_directories(meter PUBLIC src/)
    target_link_directories(meter PUBLIC /opt/gcc-msp430-none/include)
    target_link_options(meter PUBLIC
            -T${PROJECT_SOURCE_DIR}/src/${MMCU}.ld
            -mmcu=${MMCU})
    target_sources(meter
            PRIVATE
            src/drivers/sh1107.cpp
            src/util/7segment.cpp
            src/msp430i2.cpp
            src/util.cpp
            PUBLIC
            src/display/draw.hpp
            src/display/readout.hpp
            src/drivers/rotary_encoder.hpp
            src/drivers/sh1107.hpp
            src/msp430i2.hpp src/msp430.hpp
            src/msp/i2c.hpp src/msp/spi.hpp src/msp/uart.hpp
            src/util/7segment.hpp
            src/adc.hpp
            src/print.hpp
            src/util.hpp)

    add_executable(meter_firmware)
    target_link_libraries(meter_firmware PRIVATE meter)
    target_link_options(meter_firmware PRIVATE
            LINKER:--gc-sections
            LINKER:-print-memory-usage
            LINKER:-Map,${PROJECT_BINARY_DIR}/meter_firmware.map)
    target_sources(meter_firmware PRIVATE
            src/main.cpp
            src/meter.cpp src/meter.hpp
            src/calibration.hpp
            src/config.hpp)

    add_custom_command(TARGET meter_firmware POST_BUILD
            COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:meter_firmware> > meter_firmware.S
            COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:meter_firmware> meter_firmware.bin)


    add_executable(ccsrc_firmware)
    target_link_libraries(ccsrc_firmware PRIVATE meter)
    target_link_options(ccsrc_firmware PRIVATE
            LINKER:--gc-sections
            LINKER:-print-memory-usage
            LINKER:-Map,${PROJECT_BINARY_DIR}/ccsrc_firmware.map)
    target_sources(ccsrc_firmware PRIVATE src/applications/ccsrc.cpp)

else () # building for host => unit tests & tooling

    find_package(Catch2 3 REQUIRED)

    add_executable(meter_unit_tests)
    target_compile_features(meter_unit_tests PRIVATE cxx_std_20)
    target_include_directories(meter_unit_tests PRIVATE src/)
    target_link_libraries(meter_unit_tests PRIVATE Catch2::Catch2WithMain)
    target_sources(meter_unit_tests PRIVATE
            src/util/7segment_test.cpp src/util/7segment.cpp src/util/7segment.hpp
            src/util.cpp src/util.hpp
            src/test.cpp)

    add_executable(statistics)
    target_compile_features(statistics PRIVATE cxx_std_20)
    target_sources(statistics PRIVATE tools/statistics.cpp)

endif ()
