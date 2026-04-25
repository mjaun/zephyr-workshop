#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/pwm.h>

#define PWM_CHANNEL 2

LOG_MODULE_REGISTER(main);

static void read_sensor(struct sensor_value* accel_z);
static void control_buzzer(const struct sensor_value *accel_z);

int main(void)
{
    LOG_INF("%s %s", CONFIG_APP_HELLO_WORLD_MSG, CONFIG_BOARD_TARGET);

    while (true) {
        k_sleep(K_MSEC(20));

        struct sensor_value accel_z = {0};
        read_sensor(&accel_z);
        control_buzzer(&accel_z);
    }

    return 0;
}

void read_sensor(struct sensor_value *accel_z)
{
    const struct device* sensor_dev = DEVICE_DT_GET(DT_ALIAS(accelerometer));
    int ret = 0;

    ret = sensor_sample_fetch(sensor_dev);

    if (ret != 0) {
        LOG_ERR("sensor_sample_fetch: %d", ret);
        return;
    }

    ret = sensor_channel_get(sensor_dev, SENSOR_CHAN_ACCEL_Z, accel_z);

    if (ret != 0) {
        LOG_ERR("sensor_channel_get: %d", ret);
        return;
    }
}

void control_buzzer(const struct sensor_value *accel_z)
{
    const struct device* buzzer_dev = DEVICE_DT_GET(DT_ALIAS(buzzer));
    int ret = 0;

    const float value = sensor_value_to_float(accel_z);

    const float min_freq = 500;
    const float max_freq = 3000;
    const float min_accel = 8;
    const float max_accel = -9;

    if (value > min_accel) {
        ret = pwm_set(
            buzzer_dev,
            PWM_CHANNEL,
            0,
            0,
            PWM_POLARITY_NORMAL
        );
    } else {
        const float freq = min_freq + (value - min_accel) * (max_freq - min_freq) / (max_accel - min_accel);
        const uint32_t period = PWM_HZ((uint32_t) freq);
        const uint32_t pulse = period / 2;

        ret = pwm_set(
            buzzer_dev,
            PWM_CHANNEL,
            period,
            pulse,
            PWM_POLARITY_NORMAL
        );
    }

    if (ret != 0) {
        LOG_ERR("pwm_set: %d", ret);
    }
}
