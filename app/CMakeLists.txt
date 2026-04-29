cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(workshop)

target_sources(app PRIVATE 
    src/drivers/buzzer.c
    src/main.c 
)

if(CONFIG_ARCH_POSIX)
    target_sources(app PRIVATE 
        src/drivers/sensor_sim.c
        src/drivers/pwm_sim.c
    )
endif()
