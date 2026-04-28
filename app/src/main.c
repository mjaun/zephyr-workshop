#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include "drivers/buzzer.h"

#define PWM_CHANNEL 2

LOG_MODULE_REGISTER(main);

static void sensor_timer_callback(struct k_timer *timer);
static void sensor_thread_function(void *p1, void *p2, void *p3);
static void buzzer_thread_function(void *p1, void *p2, void *p3);

K_TIMER_DEFINE(sensor_timer, sensor_timer_callback, NULL);
K_SEM_DEFINE(sensor_sem, 0, 1);
K_THREAD_DEFINE(sensor_thread, 1024, sensor_thread_function, NULL, NULL, NULL, 5, 0, 0);
K_MSGQ_DEFINE(message_queue, sizeof(struct sensor_value), 3, 1);
K_THREAD_DEFINE(buzzer_thread, 1024, buzzer_thread_function, NULL, NULL, NULL, 5, 0, 0);

int main(void)
{
    LOG_INF("%s %s", CONFIG_APP_HELLO_WORLD_MSG, CONFIG_BOARD_TARGET);

    k_timer_start(&sensor_timer, K_MSEC(20), K_MSEC(20));

    return 0;
}

static void sensor_timer_callback(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    k_sem_give(&sensor_sem);
}

static void sensor_thread_function(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    const struct device* sensor_dev = DEVICE_DT_GET(DT_ALIAS(accelerometer));
    struct sensor_value accel_z;
    int ret = 0;

    while (true) {
        k_sem_take(&sensor_sem, K_FOREVER);

        ret = sensor_sample_fetch(sensor_dev);

        if (ret != 0) {
            LOG_ERR("sensor_sample_fetch: %d", ret);
            continue;
        }

        ret = sensor_channel_get(sensor_dev, SENSOR_CHAN_ACCEL_Z, &accel_z);

        if (ret != 0) {
            LOG_ERR("sensor_channel_get: %d", ret);
            continue;
        }

        ret = k_msgq_put(&message_queue, &accel_z, K_NO_WAIT);

        if (ret != 0) {
            LOG_WRN("k_msgq_put: %d", ret);
        }
    }
}

static void buzzer_thread_function(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    const struct device* buzzer_dev = DEVICE_DT_GET(DT_ALIAS(buzzer));
    struct sensor_value accel_z;
    int ret = 0;

    while (true) {
        ret = k_msgq_get(&message_queue, &accel_z, K_FOREVER);

        if (ret != 0) {
            LOG_ERR("k_msgq_get: %d", ret);
            continue;
        }

        const float min_freq = 500;
        const float max_freq = 3000;
        const float min_accel = 8;
        const float max_accel = -9;
        const float value = sensor_value_to_float(&accel_z);

        if (value > min_accel) {
            ret = buzzer_disable(buzzer_dev);
        } else {
            const float freq = min_freq + (value - min_accel) * (max_freq - min_freq) / (max_accel - min_accel);
            ret = buzzer_enable(buzzer_dev, (uint32_t) freq);
        }

        if (ret != 0) {
            LOG_ERR("pwm_set: %d", ret);
        }
    }
}
