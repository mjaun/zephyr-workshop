#include "SensorReader.hpp"

SensorReader::SensorReader(Thread &thread, MessageQueue<float> &message_queue, Accelerometer &accelerometer) : 
    _thread(thread),
    _message_queue(message_queue),
    _accelerometer(accelerometer),
    _timer([this] { timer_callback(); }),
    _semaphore(0, 1)
{
}

void SensorReader::start()
{
    _thread.start([this] { thread_main(); });
    _timer.start(std::chrono::milliseconds(20));
}

void SensorReader::timer_callback()
{
    _semaphore.give();
}

void SensorReader::thread_main()
{
    while (true) {
        _semaphore.take();
        const float value = _accelerometer.read();
        _message_queue.put(value);
    }
}
