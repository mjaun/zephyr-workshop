#pragma once

#include "ApplicationAssert.hpp"
#include <zephyr/kernel.h>
#include <functional>

class Thread
{
public:
    using EntryFn = std::function<void()>;

    Thread(k_thread_stack_t* stack, size_t stack_size, int prio)
    {
        _tid = k_thread_create(
            &_thread, 
            stack, stack_size, 
            entry_wrapper, 
            this, nullptr, nullptr, 
            prio, 0, K_FOREVER
        );
    }

    ~Thread()
    {
        k_thread_abort(_tid);
    }

    void start(EntryFn entry)
    {
        APP_ASSERT(!_entry);
        _entry = entry;
        k_thread_start(_tid);
    }

    Thread(const Thread &) = delete;
    Thread &operator=(const Thread &) = delete;
    Thread(Thread &&) = delete;
    Thread &operator=(Thread &&) = delete;

private:
    struct k_thread _thread;
    k_tid_t _tid;
    EntryFn _entry;

    static void entry_wrapper(void *arg1, void *arg2, void *arg3)
    {
        ARG_UNUSED(arg2);
        ARG_UNUSED(arg3);
        reinterpret_cast<Thread *>(arg1)->_entry();
    }
};
