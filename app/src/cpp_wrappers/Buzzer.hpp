#pragma once

#include "ApplicationAssert.hpp"
#include "drivers/buzzer.h"

class IBuzzer
{
public:
    virtual ~IBuzzer() = default;

    virtual void enable(uint32_t freq) = 0;
    virtual void disable() = 0;
};

class Buzzer : public IBuzzer
{
public:
    Buzzer(const struct device *device) : _device(device) {}

    void enable(uint32_t freq)
    {
        int ret = buzzer_enable(_device, freq);
        APP_ASSERT(ret == 0);
    }

    void disable()
    {
        int ret = buzzer_disable(_device);
        APP_ASSERT(ret == 0);
    }

    Buzzer(const Buzzer &) = delete;
    Buzzer &operator=(const Buzzer &) = delete;
    Buzzer(Buzzer &&) = delete;
    Buzzer &operator=(Buzzer &&) = delete;

private:
    const struct device *_device;
};
