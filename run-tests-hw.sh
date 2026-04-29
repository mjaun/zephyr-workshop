#!/bin/bash
set -euo pipefail

# this script executes application test suites on the actual hardware
# it must be executed within the dev container

west twister \
    --device-testing \
    --device-serial /dev/ttyACM0 \
    --board-root /workdir/boards \
    --extra-args BOARD_ROOT=/workdir \
    --west-runner pyocd \
    --platform demo \
    --testsuite-root app/tests
