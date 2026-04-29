#!/bin/bash
set -euo pipefail

# this script executes application test suites using the native simulator
# it must be executed within the dev container

west twister \
    --board-root /workdir/boards \
    --platform native_sim \
    --testsuite-root app/tests
