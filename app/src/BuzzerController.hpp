#pragma once

#include "cpp_wrappers/Thread.hpp"
#include "cpp_wrappers/SoftwareTimer.hpp"
#include "cpp_wrappers/Semaphore.hpp"
#include "cpp_wrappers/MessageQueue.hpp"
#include "cpp_wrappers/Buzzer.hpp"

class BuzzerController
{
public:
    BuzzerController(Thread &thread, MessageQueue<float> &message_queue, IBuzzer &Buzzer);

    void start();

    BuzzerController(const BuzzerController &) = delete;
    BuzzerController &operator=(const BuzzerController &) = delete;
    BuzzerController(BuzzerController &&) = delete;
    BuzzerController &operator=(BuzzerController &&) = delete;

private:
    Thread &_thread;
    MessageQueue<float> &_message_queue;
    IBuzzer &_buzzer;

    void thread_main();
};
