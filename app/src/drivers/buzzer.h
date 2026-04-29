#pragma once

#include <zephyr/device.h>

#ifdef __cplusplus
extern "C" {
#endif

int buzzer_enable(const struct device *dev, uint32_t freq);
int buzzer_disable(const struct device *dev);

#ifdef __cplusplus
}
#endif
