*** Settings ***
Resource                      ${KEYWORDS}

*** Variables ***
# TIM3 (buzzer PWM, CH2) registers; timer clock is 1 MHz (prescaler 95 @ 96 MHz)
${TIM3_CR1}                   0x40000400
${TIM3_CCER}                  0x40000420
${TIM3_ARR}                   0x4000042C
${TIM3_CCR2}                  0x40000438
${TIM3_CLOCK_HZ}              1000000
${CCER_CC2E}                  0x10
${TOLERANCE}                  0.02
${RENODE_DIR}                 ${CURDIR}/../../../renode

*** Keywords ***
Prepare Demo Machine
    [Documentation]           Like "Prepare Machine" (twister's common.robot) but first points demo.resc at the Renode platform files in the repository.
    Execute Command           $repl = @${RENODE_DIR}/stm32u385.repl
    Execute Command           $lsm6dso16is_cs = @${RENODE_DIR}/lsm6dso16is.cs
    Execute Command           $stm32_i2c_cs = @${RENODE_DIR}/stm32_i2c.cs
    Execute Command           $stm32u3_stubs_cs = @${RENODE_DIR}/stm32u3_stubs.cs
    Prepare Machine

Boot Application
    [Documentation]           Loads the ELF via the twister-provided ${RESC}, starts the emulation and waits for the boot banner.
    Prepare Demo Machine
    Start Emulation
    Wait For Line On Uart     Hello World! demo
    Execute Command           pause

Set Acceleration Z
    [Documentation]           Inject Z acceleration [m/s^2] into the simulated LSM6DSO16IS and let the application run for a few sampling periods.
    [Arguments]               ${mps2}
    Execute Command           sysbus.i2c2.lsm6dso16is SetAccelZ ${mps2}
    Execute Command           emulation RunFor "0.2"

Read Register
    [Documentation]           Reads a register of the simulated machine.
    [Arguments]               ${address}
    ${output}=                Execute Command    sysbus ReadDoubleWord ${address}
    ${text}=                  Strip String       ${output}
    ${value}=                 Convert To Integer    ${text}
    RETURN                    ${value}

Buzzer Should Be Off
    ${ccer}=                  Read Register      ${TIM3_CCER}
    ${enabled}=               Evaluate           ${ccer} & ${CCER_CC2E}
    Should Be Equal As Integers    ${enabled}    0    Buzzer output enabled but expected off (CCER=${ccer})

Buzzer Frequency Should Be
    [Arguments]               ${expected_hz}
    ${ccer}=                  Read Register      ${TIM3_CCER}
    ${enabled}=               Evaluate           ${ccer} & ${CCER_CC2E}
    Should Not Be Equal As Integers    ${enabled}    0    Buzzer output disabled but expected ${expected_hz} Hz
    ${arr}=                   Read Register      ${TIM3_ARR}
    ${ccr}=                   Read Register      ${TIM3_CCR2}
    ${freq}=                  Evaluate           ${TIM3_CLOCK_HZ} / (${arr} + 1)
    ${error}=                 Evaluate           abs(${freq} - ${expected_hz}) / ${expected_hz}
    Should Be True            ${error} < ${TOLERANCE}    Buzzer at ${freq} Hz, expected ${expected_hz} Hz
    ${duty_error}=            Evaluate           abs(${ccr} * 2 - (${arr} + 1))
    Should Be True            ${duty_error} <= 2    Duty is not 50% (ARR=${arr}, CCR2=${ccr})

Expected Frequency For Z
    [Documentation]           Mirrors the mapping in app/src/buzzer_node.c: 8 m/s^2 -> 500 Hz, -9 m/s^2 -> 3000 Hz (linear).
    [Arguments]               ${mps2}
    ${hz}=                    Evaluate           500 + (${mps2} - 8) * (3000 - 500) / (-9 - 8)
    RETURN                    ${hz}

Buzzer Should Follow Z
    [Arguments]               ${mps2}
    Set Acceleration Z        ${mps2}
    ${hz}=                    Expected Frequency For Z    ${mps2}
    Buzzer Frequency Should Be    ${hz}
