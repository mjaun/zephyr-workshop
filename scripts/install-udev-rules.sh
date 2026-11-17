#!/bin/bash
set -e

# this script installs udev rules to let the regular user access USB debuggers
# it must be executed with sudo in a WSL terminal

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cp "$SCRIPT_DIR/60-openocd.rules" /etc/udev/rules.d/
udevadm control --reload
