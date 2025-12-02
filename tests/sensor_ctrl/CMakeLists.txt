cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(measurement)

set(APP_SRC ${ZEPHYR_WORKSHOP_MODULE_DIR}/app/src)

zephyr_include_directories(${APP_SRC})

target_sources(app PRIVATE
    src/main.c
    src/sensor_fake.c
    ${APP_SRC}/sensor_ctrl.c
)
