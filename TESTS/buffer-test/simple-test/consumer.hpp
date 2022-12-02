#pragma once

#include "mbed.h"

#include "mbed_trace.h"
#if MBED_CONF_MBED_TRACE_ENABLE
#define TRACE_GROUP "Producer"
#endif // MBED_CONF_MBED_TRACE_ENABLE

#include "buffer.hpp"

class Consumer {
public:
    Consumer(Buffer &buffer, const std::chrono::microseconds &consumeWaitTime, uint32_t nbrOfValues) :
        _buffer(buffer),
        _consumeWaitTime(consumeWaitTime),
        _nbrOfValues(nbrOfValues),
        _consumerThread(osPriorityNormal, OS_STACK_SIZE, nullptr, "ConsumerThread")
    {
        // initialize random seed
        srand(time(NULL));
    }

    void start()
    {
        _consumerThread.start(callback(this, &Consumer::consumerMethod));
    }

    void wait()
    {
        _consumerThread.join();
    }

private:

    void consume(int datum)
    {
        // wait for a randomized consumer wait time
        wait_us(computeRandomWaitTime(_consumeWaitTime));
    }

    int computeRandomWaitTime(const std::chrono::microseconds &waitTime)
    {
        return rand() % waitTime.count() + waitTime.count();
    }

    void consumerMethod(void)
    {
        uint32_t index = 0;
        while (index < _nbrOfValues) {
            uint32_t consumerDatum = _buffer.extract();            
            consume(consumerDatum);
            tr_debug("Extracting data at index %" PRIu32 " (data  %" PRIu32 ")\n", index, consumerDatum);
            index++;
        }
    }

private:
    Buffer &_buffer;
    const std::chrono::microseconds &_consumeWaitTime;
    uint32_t _nbrOfValues;
    Thread _consumerThread;
};
