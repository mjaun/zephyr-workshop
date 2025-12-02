cmake_minimum_required(VERSION 3.20.0)

set(BOARD demo)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(workshop)

target_sources(app PRIVATE
    src/main.c
    src/sensor_ctrl.c
    src/buzzer_ctrl.c
    src/drivers/buzzer.c
)
