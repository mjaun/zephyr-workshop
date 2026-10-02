#!/bin/bash
set -e

# this script installs udev rules to let the regular user access USB debuggers
# it must be executed with sudo in a WSL terminal

wget -O 60-openocd.rules https://sf.net/p/openocd/code/ci/master/tree/contrib/60-openocd.rules?format=raw
cp 60-openocd.rules /etc/udev/rules.d && rm 60-openocd.rules
udevadm control --reload
