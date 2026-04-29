cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(test_controller_reader)

execute_process(
    COMMAND west topdir 
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} 
    OUTPUT_VARIABLE WEST_ROOT 
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

add_subdirectory(${WEST_ROOT}/external/googletest googletest)

target_link_libraries(app PRIVATE GTest::gmock GTest::gtest)
target_compile_definitions(app PRIVATE GTEST_HAS_PTHREAD=0)

target_include_directories(app PRIVATE ${WEST_ROOT}/app/src)

target_sources(app PRIVATE
    ${WEST_ROOT}/app/src/cpp_wrappers/ApplicationAssert.cpp
    ${WEST_ROOT}/app/src/SensorReader.cpp
    ${WEST_ROOT}/app/src/BuzzerController.cpp
    ${WEST_ROOT}/app/src/frequency.c
    src/BuzzerControllerTest.cpp
    src/main.cpp
)
