#pragma once

#include <zephyr/kernel.h>
#include "ApplicationAssert.hpp"

template <typename T, unsigned int max_msgs>
class MessageQueue
{
public:
    MessageQueue()
    {
        k_msgq_init(&_msgq, _buf, sizeof(T), max_msgs);
    }

    void put(const T &item)
    {
        int ret = k_msgq_put(&_msgq, &item, K_NO_WAIT);
        APP_ASSERT(ret == 0);
    }

    T get()
    {
        T item;
        int ret = k_msgq_get(&_msgq, &item, K_FOREVER);
        APP_ASSERT(ret == 0);
        return item;
    }

    MessageQueue(const MessageQueue &) = delete;
    MessageQueue &operator=(const MessageQueue &) = delete;
    MessageQueue(MessageQueue &&) = delete;
    MessageQueue &operator=(MessageQueue &&) = delete;

private:
    struct k_msgq _msgq;
    char _buf[sizeof(T) * max_msgs];
};
