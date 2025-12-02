#include <zephyr/logging/log.h>
#include "sensor_ctrl.h"

LOG_MODULE_REGISTER(main);

int main(void)
{
    LOG_INF("Hello World! %s", CONFIG_BOARD_TARGET);

    sensor_ctrl_start();

    return 0;
}

