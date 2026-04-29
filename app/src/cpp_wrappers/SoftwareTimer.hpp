#pragma once

#include <zephyr/kernel.h>
#include <functional>
#include <chrono>

class SoftwareTimer
{
public:
    using CallbackFn = std::function<void()>;

    SoftwareTimer(CallbackFn callback) : _callback(callback)
    {
        k_timer_init(&_timer, callback_wrapper, NULL);
        k_timer_user_data_set(&_timer, this);
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
        auto *instance = reinterpret_cast<SoftwareTimer *>(k_timer_user_data_get(timer));
        instance->_callback();
    }
};
