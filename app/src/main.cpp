#include "SensorReader.hpp"
#include "BuzzerController.hpp"
#include "cpp_wrappers/Buzzer.hpp"
#include "cpp_wrappers/Accelerometer.hpp"
#include "cpp_wrappers/Thread.hpp"
#include "cpp_wrappers/MessageQueue.hpp"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

K_THREAD_STACK_DEFINE(sensor_thread_stack, 1024);
K_THREAD_STACK_DEFINE(buzzer_thread_stack, 1024);

static Thread sensor_thread(sensor_thread_stack, K_THREAD_STACK_SIZEOF(sensor_thread_stack), 5);
static Thread buzzer_thread(buzzer_thread_stack, K_THREAD_STACK_SIZEOF(buzzer_thread_stack), 5);

static MessageQueue<float> message_queue(3);

static Accelerometer accelerometer(DEVICE_DT_GET(DT_ALIAS(accelerometer)));
static Buzzer buzzer(DEVICE_DT_GET(DT_ALIAS(buzzer)));

static SensorReader sensor_reader(sensor_thread, message_queue, accelerometer);
static BuzzerController buzzer_controller(buzzer_thread, message_queue, buzzer);

extern "C" int main(void)
{
    LOG_INF("%s %s", CONFIG_APP_HELLO_WORLD_MSG, CONFIG_BOARD_TARGET);

    sensor_reader.start();
    buzzer_controller.start();

    return 0;
}
