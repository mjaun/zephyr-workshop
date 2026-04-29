#include "cpp_wrappers/Buzzer.hpp"
#include "cpp_wrappers/Accelerometer.hpp"
#include "cpp_wrappers/Thread.hpp"
#include "cpp_wrappers/SoftwareTimer.hpp"
#include "cpp_wrappers/MessageQueue.hpp"
#include "cpp_wrappers/Semaphore.hpp"
#include <zephyr/logging/log.h>
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
static MessageQueue<float, 3> message_queue;

static Accelerometer accelerometer(DEVICE_DT_GET(DT_ALIAS(accelerometer)));
static Buzzer buzzer(DEVICE_DT_GET(DT_ALIAS(buzzer)));

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
    while (true) {
        sensor_sem.take();
        const float value = accelerometer.read();
        message_queue.put(value);
    }
}

static void buzzer_thread_function()
{
    while (true) {
        const float value = message_queue.get();
        const uint32_t freq = accel_to_freq(value);

        if (freq == 0) {
            buzzer.disable();
        } else {
            buzzer.enable(freq);
        }
    }
}
