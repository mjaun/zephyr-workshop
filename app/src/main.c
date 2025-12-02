#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/pwm.h>

LOG_MODULE_REGISTER(main);

static void sensor_timer_callback(struct k_timer *timer);
static void sensor_thread_function(void *p1, void *p2, void *p3);
static void buzzer_thread_function(void *p1, void *p2, void *p3);

K_TIMER_DEFINE(sensor_timer, sensor_timer_callback, NULL);
K_SEM_DEFINE(sensor_sem, 0, 1);
K_THREAD_DEFINE(sensor_thread, 1024, sensor_thread_function, NULL, NULL, NULL, 5, 0, 0);
K_MSGQ_DEFINE(message_queue, sizeof(int), 3, 1);
K_THREAD_DEFINE(buzzer_thread, 1024, buzzer_thread_function, NULL, NULL, NULL, 5, 0, 0);

int main(void)
{
    LOG_INF("Hello World! %s", CONFIG_BOARD_TARGET);

    k_timer_start(&sensor_timer, K_MSEC(10), K_MSEC(10));

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

    const struct device* sensor_dev = DEVICE_DT_GET(DT_CHOSEN(workshop_sensor));
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

    const struct device* buzzer_dev = DEVICE_DT_GET(DT_CHOSEN(workshop_buzzer));
    struct sensor_value accel_z;
    int ret = 0;

    while (true) {
        ret = k_msgq_get(&message_queue, &accel_z, K_FOREVER);

        if (ret != 0) {
            LOG_ERR("k_msgq_get: %d", ret);
            continue;
        }

        const uint32_t pulse_usec = (accel_z.val1 >= 9) ? 0 : 200;

        ret = pwm_set(
            buzzer_dev,
            2,
            PWM_USEC(1000),
            PWM_USEC(pulse_usec),
            PWM_POLARITY_NORMAL
        );

        if (ret != 0) {
            LOG_ERR("pwm_set: %d", ret);
        }
    }
}
