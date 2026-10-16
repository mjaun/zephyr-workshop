#!/bin/bash
set -euo pipefail

# this script executes the Robot tests on the demo firmware image simulated in Renode
# it must be executed within the dev container

west twister \
    --board-root /workdir/boards \
    --platform demo \
    --tag renode \
    --extra-args BOARD_ROOT=/workdir \
    --testsuite-root app
