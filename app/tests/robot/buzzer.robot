*** Settings ***
Resource                      ${KEYWORDS}
Resource                      ${CURDIR}/buzzer_keywords.resource
Test Teardown                 Test Teardown

*** Test Cases ***
Should Boot Without Errors
    Prepare Demo Machine
    Start Emulation
    Wait For Line On Uart     Hello World! demo
    Should Not Be On Uart     <err>    timeout=1

Buzzer Should Be Off When Device Is Flat
    Boot Application
    Set Acceleration Z        9.81
    Buzzer Should Be Off

Buzzer Should Be Off Above Threshold
    Boot Application
    Set Acceleration Z        8.5
    Buzzer Should Be Off

Buzzer Should Sound 500 Hz At Threshold
    Boot Application
    Buzzer Should Follow Z    7.9

Buzzer Frequency Should Rise While Tilting Over
    Boot Application
    Buzzer Should Follow Z    4.0
    Buzzer Should Follow Z    0.0
    Buzzer Should Follow Z    -5.0
    Buzzer Should Follow Z    -9.0

Buzzer Should Sound 3 kHz At Minus 9
    Boot Application
    Buzzer Should Follow Z    -9.0
    Buzzer Frequency Should Be    3000

Buzzer Should Stop When Returned To Flat
    Boot Application
    Buzzer Should Follow Z    -9.0
    Set Acceleration Z        9.81
    Buzzer Should Be Off
