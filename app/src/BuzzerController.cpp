#include "BuzzerController.hpp"
#include "frequency.h"

BuzzerController::BuzzerController(Thread &thread, 
                                   MessageQueue<float> &message_queue, 
                                   IBuzzer &buzzer) :
    _thread(thread),
    _message_queue(message_queue),
    _buzzer(buzzer)
{
}

void BuzzerController::start()
{
    _thread.start([this] { thread_main(); });
}

void BuzzerController::thread_main()
{
    while (true) {
        const float value = _message_queue.get();
        const uint32_t freq = accel_to_freq(value);

        if (freq == 0) {
            _buzzer.disable();
        } else {
            _buzzer.enable(freq);
        }
    }
}
