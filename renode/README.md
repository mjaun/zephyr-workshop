# Renode simulation of the `demo` firmware

The real `demo` (STM32U385) firmware image is run in [Renode](https://renode.io) and tested with
[Robot Framework](https://robotframework.org). Everything is launched through twister:

    ./scripts/run-tests-renode.sh

Twister builds the application itself (`app/sample.yaml`, so no separate CMake file) for `demo` and runs
`renode-test` on `buzzer.robot`. Outputs: `twister-out/demo_stm32u385xx/.../app.renode.buzzer/`
(`handler.log`, `log.html`, `report.html`).

## Running without twister

Build the firmware, then call `renode-test` with the variables twister would pass
(`KEYWORDS` from Zephyr, `RESC`/`UART` from `boards/demo/demo.yaml`):

    west build -b demo -d app/build/demo app -- -DBOARD_ROOT=/workdir

    renode-test \
      --variable KEYWORDS:/workdir/external/zephyr/tests/robot/common.robot \
      --variable ELF:@/workdir/app/build/demo/zephyr/zephyr.elf \
      --variable RESC:@/workdir/renode/demo.resc \
      --variable UART:sysbus.usart1 \
      -r robot-results \
      app/tests/robot/buzzer.robot

All four variables are required; the defaults in `demo.resc` only apply when starting Renode by hand
(`renode renode/demo.resc`). Add `-t "<test name>"` to run a single test. Results (`log.html`,
`report.html`, `robot_output.xml`) go to `robot-results/`. Unlike twister, they list every Robot
test individually; twister reports the whole `.robot` file as one test case.

## Renode harness

| File | Purpose |
|------|---------|
| `boards/demo/demo.yaml` | declares `simulation: renode` and the `resc`/`uart` used by twister |
| `renode/demo.resc` | creates the machine and loads the ELF |
| `renode/stm32u385.repl` | platform description (CPU, memories, USART1, I2C2, TIM3, GPIO) |
| `renode/*.cs` | C# peripherals: I2C controller, LSM6DSO16IS accelerometer, RCC/PWR stubs |
| `app/tests/robot/` | twister suite is `app/sample.yaml`; Robot tests and keywords |

Tests inject accelerations through `sysbus.i2c2.lsm6dso16is SetAccelZ` and check the buzzer by
reading the TIM3 registers (ARR/CCR2/CCER). The `.cs` peripherals are minimal models, only as
complete as this application needs.

To add a test, add a case to `buzzer.robot` (or a new `.robot` file listed in `robot_testsuite`).
Each Robot file counts as one twister test case.
