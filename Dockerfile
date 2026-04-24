FROM ghcr.io/zephyrproject-rtos/zephyr-build:v0.28.7

# Dependency for serial monitor extension
RUN sudo apt update \
 && sudo apt install -y udev \
 && sudo apt clean

# PyOCD package for STM32U3
RUN pyocd pack --update && pyocd pack --install STM32U3 # PyOCD package for STM32U3
