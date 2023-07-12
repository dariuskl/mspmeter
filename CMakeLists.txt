cmake_minimum_required(VERSION 3.13)

project(mspmeter VERSION 0.1.0.0
        DESCRIPTION "firmware for a MSP430i2x-based digital voltage and current meter"
        LANGUAGES CXX ASM)


if (NOT (${CMAKE_SYSTEM_PROCESSOR} STREQUAL ${CMAKE_HOST_SYSTEM_PROCESSOR}))
    # build firmware only when compiling for target

    set(MMCU "msp430i2041")

    add_compile_options(-Werror -Wall -Wextra -pedantic -pedantic-errors
                        -Wconversion -Wsign-conversion -Wreorder
                        -flto -ffunction-sections -fdata-sections
                        -fno-rtti -fno-exceptions
                        -fconcepts
                        -gstabs
                        -Os
                        -mmcu=${MMCU})


    add_executable(meter_firmware)

    set_target_properties(meter_firmware PROPERTIES
                          LINK_DEPENDS ${PROJECT_SOURCE_DIR}/src/${MMCU}.ld)

    target_compile_features(meter_firmware PRIVATE cxx_std_20)

    target_include_directories(meter_firmware PRIVATE src/)

    target_link_directories(meter_firmware PRIVATE /opt/gcc-msp430-none/include)

    target_link_options(meter_firmware PRIVATE LINKER:--gc-sections
                        LINKER:-print-memory-usage
                        LINKER:-Map,${PROJECT_BINARY_DIR}/meter_firmware.map
                        -T${PROJECT_SOURCE_DIR}/src/${MMCU}.ld
                        -mmcu=${MMCU})

    target_sources(meter_firmware PRIVATE
            src/main.cpp
            src/meter.cpp src/meter.hpp
            src/msp430i2.cpp src/msp430i2.hpp src/msp430.hpp
            src/msp/spi.hpp src/msp/uart.hpp
            src/adc.hpp
            src/calibration.hpp
            src/config.hpp
            src/future.hpp
            src/readout.hpp
            src/rotary_encoder.hpp
            src/util.cpp src/util.hpp)

    add_custom_command(TARGET meter_firmware POST_BUILD
                       COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:meter_firmware> > meter_firmware.S
                       COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:meter_firmware> meter_firmware.bin)

else () # building for host => unit tests

    find_package(Catch2 3 REQUIRED)

    add_executable(meter_unit_tests)
    target_compile_features(meter_unit_tests PRIVATE cxx_std_20)
    target_link_libraries(meter_unit_tests PRIVATE Catch2::Catch2WithMain)
    target_sources(meter_unit_tests PRIVATE src/util.cpp src/test.cpp)

endif ()
