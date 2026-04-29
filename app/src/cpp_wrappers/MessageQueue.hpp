#pragma once

#include "ApplicationAssert.hpp"
#include <zephyr/kernel.h>
#include <memory>

template <typename T>
class MessageQueue
{
public:
    MessageQueue(uint32_t max_msgs)
    {
        _buf = std::make_unique<char[]>(sizeof(T) * max_msgs);
        k_msgq_init(&_msgq, _buf.get(), sizeof(T), max_msgs);
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
    std::unique_ptr<char[]> _buf;
};
