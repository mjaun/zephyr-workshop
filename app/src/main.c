#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

int main(void)
{
    LOG_INF("Hello World! %s", CONFIG_BOARD_TARGET);
    return 0;
}
