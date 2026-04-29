#pragma once

#include <zephyr/kernel.h>
#include <chrono>

class SoftwareTimer
{
public:
    using CallbackFn = void (*)();

    SoftwareTimer(CallbackFn callback) : _callback(callback)
    {
        k_timer_init(&_timer, callback_wrapper, NULL);
    }

    void start(std::chrono::milliseconds period)
    {
        k_timer_start(&_timer, K_MSEC(period.count()), K_MSEC(period.count()));
    }

    SoftwareTimer(const SoftwareTimer &) = delete;
    SoftwareTimer &operator=(const SoftwareTimer &) = delete;
    SoftwareTimer(SoftwareTimer &&) = delete;
    SoftwareTimer &operator=(SoftwareTimer &&) = delete;

private:
    struct k_timer _timer;
    const CallbackFn _callback;

    static void callback_wrapper(struct k_timer* timer)
    {
        CONTAINER_OF(timer, SoftwareTimer, _timer)->_callback();
    }
};
