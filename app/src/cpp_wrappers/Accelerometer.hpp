#pragma once

#include "ApplicationAssert.hpp"
#include <zephyr/drivers/sensor.h>

class Accelerometer
{
public:
    Accelerometer(const struct device *device) : _device(device) {}

    float read()
    {
        struct sensor_value value = {};
        int ret;

        ret = sensor_sample_fetch(_device);
        APP_ASSERT(ret == 0);

        ret = sensor_channel_get(_device, SENSOR_CHAN_ACCEL_Z, &value);
        APP_ASSERT(ret == 0);

        return sensor_value_to_float(&value);
    }

    Accelerometer(const Accelerometer &) = delete;
    Accelerometer &operator=(const Accelerometer &) = delete;
    Accelerometer(Accelerometer &&) = delete;
    Accelerometer &operator=(Accelerometer &&) = delete;

private:
    const struct device *_device;
};
