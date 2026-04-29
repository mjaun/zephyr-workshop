#include "ApplicationAssert.hpp"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

LOG_MODULE_REGISTER(assert);

void assert_failed(const char* file_name, unsigned int line)
{
    LOG_ERR("Assertion failed: %s:%d", file_name, line);
    k_panic();
}
