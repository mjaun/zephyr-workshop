#include <zephyr/kernel.h>
#include <app_version.h>

#include "serial_dfu.h"

int main(void)
{
    printk("Hello World! %s v%s\n", CONFIG_BOARD_TARGET, APP_VERSION_STRING);

    serial_dfu_start();
    return 0;
}
