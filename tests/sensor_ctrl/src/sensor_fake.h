#pragma once

#include <zephyr/fff.h>

DECLARE_FAKE_VALUE_FUNC(int, fake_sensor_sample_fetch, const struct device *, enum sensor_channel);

DECLARE_FAKE_VALUE_FUNC(int, fake_sensor_channel_get, const struct device *, enum sensor_channel,
                        struct sensor_value *);
