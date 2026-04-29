#pragma once

#include <zephyr/kernel.h>

class Semaphore
{
public:
    Semaphore(unsigned int initial_count, unsigned int limit)
    {
        k_sem_init(&_sem, initial_count, limit);
    }

    void give()
    {
        k_sem_give(&_sem);
    }

    void take()
    {
        (void)k_sem_take(&_sem, K_FOREVER);
    }

    Semaphore(const Semaphore &) = delete;
    Semaphore &operator=(const Semaphore &) = delete;
    Semaphore(Semaphore &&) = delete;
    Semaphore &operator=(Semaphore &&) = delete;

private:
    struct k_sem _sem;
};
