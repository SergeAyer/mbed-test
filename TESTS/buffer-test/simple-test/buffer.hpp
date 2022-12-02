#pragma once

#include "mbed.h"

class Buffer {
public:
    Buffer()
    {
    }

    void append(uint32_t datum)
    {
        _producerConsumerQueue.try_put_for(Kernel::wait_for_u32_forever, (uint32_t *) datum);
    }

    uint32_t extract(void)
    {
        uint32_t datum = 0;
        _producerConsumerQueue.try_get_for(Kernel::wait_for_u32_forever, (uint32_t **) &datum);
        return datum;
    }

private:
    static constexpr uint32_t kBufferSize = 10;
    Queue<uint32_t, kBufferSize> _producerConsumerQueue;
};
