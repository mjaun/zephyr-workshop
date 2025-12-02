#include "sensor_ctrl.h"
#include "buzzer_ctrl.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>

LOG_MODULE_REGISTER(sensor_ctrl);

static void sensor_timer_callback(struct k_timer *timer);
static void sensor_thread_function(void *p1, void *p2, void *p3);

K_TIMER_DEFINE(sensor_timer, sensor_timer_callback, NULL);
K_SEM_DEFINE(sensor_sem, 0, 1);
K_THREAD_DEFINE(sensor_thread, 1024, sensor_thread_function, NULL, NULL, NULL, 5, 0, 0);

void sensor_ctrl_start(void)
{
    k_timer_start(&sensor_timer, K_MSEC(10), K_MSEC(10));
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

    const struct device *sensor_dev = DEVICE_DT_GET(DT_CHOSEN(workshop_sensor));
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

        buzzer_ctrl_post(&accel_z);
    }
}
