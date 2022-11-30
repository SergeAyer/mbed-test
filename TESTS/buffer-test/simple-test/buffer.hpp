#pragma once

#include "mbed.h"

class Buffer {
public:
    Buffer()
    {
    }

    void append(int datum)
    {
        _producerConsumerQueue.try_put_for(Kernel::wait_for_u32_forever, (int *) datum);
    }

    int extract(void)
    {
        int datum = 0;
        _producerConsumerQueue.try_get_for(Kernel::wait_for_u32_forever, (int **) &datum);
        return datum;
    }

private:
    static constexpr uint32_t kBufferSize = 10;
    Queue<int, kBufferSize> _producerConsumerQueue;
};
