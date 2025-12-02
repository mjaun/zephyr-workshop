#include <zephyr/logging/log.h>
#include "posix_board_if.h"

LOG_MODULE_REGISTER(main);

int main(void)
{
    for (int i = 0; i < 10; i++) {
        LOG_INF("Hello World! %s", CONFIG_BOARD_TARGET);
        k_sleep(K_SECONDS(1));
    }

    posix_exit(0);
    return 0;
}
