#!/bin/bash

# this script attaches the Nucleo board to WSL
# it must be executed in a WSL terminal

usbipd="/mnt/c/Program Files/usbipd-win/usbipd.exe"

"$usbipd" attach --wsl --hardware-id 0483:3754 # Nucleo U385RG-Q

