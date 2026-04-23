#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

int main(void)
{
    while (true) {
        LOG_INF("Hello World! %s", CONFIG_BOARD_TARGET);
        k_sleep(K_MSEC(1000));
    }

    return 0;
}
