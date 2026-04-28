#pragma once

#include <zephyr/device.h>

int buzzer_enable(const struct device *dev, uint32_t freq);
int buzzer_disable(const struct device *dev);
