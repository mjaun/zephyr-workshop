#include "cpp_wrappers/Thread.hpp"
#include "cpp_wrappers/SoftwareTimer.hpp"
#include "cpp_wrappers/MessageQueue.hpp"
#include "cpp_wrappers/Semaphore.hpp"
#include <zephyr/logging/log.h>
#include <zephyr/drivers/sensor.h>
#include "drivers/buzzer.h"
#include "frequency.h"

#define PWM_CHANNEL 2

LOG_MODULE_REGISTER(main);

static void sensor_timer_callback();
static void sensor_thread_function();
static void buzzer_thread_function();

K_THREAD_STACK_DEFINE(sensor_thread_stack, 1024);
K_THREAD_STACK_DEFINE(buzzer_thread_stack, 1024);

static Thread sensor_thread(sensor_thread_stack, K_THREAD_STACK_SIZEOF(sensor_thread_stack), sensor_thread_function, 5);
static Thread buzzer_thread(buzzer_thread_stack, K_THREAD_STACK_SIZEOF(buzzer_thread_stack), buzzer_thread_function, 5);

static SoftwareTimer sensor_timer(sensor_timer_callback);
static Semaphore sensor_sem(0, 1);
static MessageQueue<struct sensor_value, 3> message_queue;

extern "C" int main(void)
{
    LOG_INF("%s %s", CONFIG_APP_HELLO_WORLD_MSG, CONFIG_BOARD_TARGET);

    sensor_thread.start();
    buzzer_thread.start();

    sensor_timer.start(std::chrono::milliseconds(20));

    return 0;
}

static void sensor_timer_callback()
{
    sensor_sem.give();
}

static void sensor_thread_function()
{
    const struct device* sensor_dev = DEVICE_DT_GET(DT_ALIAS(accelerometer));
    struct sensor_value accel_z;
    int ret = 0;

    while (true) {
        sensor_sem.take();

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

        message_queue.put(accel_z);
    }
}

static void buzzer_thread_function()
{
    const struct device* buzzer_dev = DEVICE_DT_GET(DT_ALIAS(buzzer));
    int ret = 0;

    while (true) {
        struct sensor_value accel_z = message_queue.get();

        const float value = sensor_value_to_float(&accel_z);
        const uint32_t freq = accel_to_freq(value);

        if (freq == 0) {
            ret = buzzer_disable(buzzer_dev);
        } else {
            ret = buzzer_enable(buzzer_dev, freq);
        }

        if (ret != 0) {
            LOG_ERR("pwm_set: %d", ret);
        }
    }
}
