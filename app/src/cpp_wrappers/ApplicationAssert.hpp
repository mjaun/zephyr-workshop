#pragma once

#include <zephyr/kernel.h>

#define APP_ASSERT(cond)                         \
    if (!(cond)) {                               \
        assert_failed(__FILE_NAME__, __LINE__);  \
    }

void assert_failed(const char* file_name, unsigned int line);
