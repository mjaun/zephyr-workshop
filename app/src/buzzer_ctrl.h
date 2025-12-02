#pragma once

#include <zephyr/drivers/sensor.h>

void buzzer_ctrl_post(const struct sensor_value* accel_z);
