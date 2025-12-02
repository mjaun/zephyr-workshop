#pragma once

#include <zephyr/device.h>

int buzzer_enable(const struct device *dev, bool enable);
