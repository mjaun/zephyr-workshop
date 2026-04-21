#!/bin/bash
set -e

# ideally these steps would installed in the used Docker image

sudo apt update && sudo apt install -y udev # dependency for serial monitor
pyocd pack --update && pyocd pack --install STM32U3 # PyOCD package for STM32U3
