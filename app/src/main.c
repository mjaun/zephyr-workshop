#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

int main(void)
{
    LOG_INF("%s %s", CONFIG_APP_HELLO_WORLD_MSG, CONFIG_BOARD_TARGET);
    return 0;
}
