cmake_minimum_required(VERSION 3.20.0)

# make the application's custom bindings (e.g. "buzzer") available
list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_ztest)

target_sources(app PRIVATE
    src/test_frequency.c
    src/test_buzzer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/frequency.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/drivers/buzzer.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/drivers
)
