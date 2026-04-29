#pragma once

#include <zephyr/kernel.h>

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
        (void)k_msgq_put(&_msgq, &item, K_FOREVER);
    }

    T get()
    {
        T item;
        (void)k_msgq_get(&_msgq, &item, K_FOREVER);
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
