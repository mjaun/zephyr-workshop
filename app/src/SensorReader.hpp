#pragma once

#include "cpp_wrappers/Thread.hpp"
#include "cpp_wrappers/SoftwareTimer.hpp"
#include "cpp_wrappers/Semaphore.hpp"
#include "cpp_wrappers/MessageQueue.hpp"
#include "cpp_wrappers/Accelerometer.hpp"

class SensorReader
{
public:
    SensorReader(Thread &thread, MessageQueue<float> &message_queue, IAccelerometer &accelerometer);

    void start();

    SensorReader(const SensorReader &) = delete;
    SensorReader &operator=(const SensorReader &) = delete;
    SensorReader(SensorReader &&) = delete;
    SensorReader &operator=(SensorReader &&) = delete;

private:
    Thread &_thread;
    MessageQueue<float> &_message_queue;
    IAccelerometer &_accelerometer;

    SoftwareTimer _timer;
    Semaphore _semaphore;

    void timer_callback();
    void thread_main();
};
