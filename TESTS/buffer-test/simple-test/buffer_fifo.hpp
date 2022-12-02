
#pragma once

class Buffer {
public:
    Buffer()
    {
    }

    void append(uint32_t datum)
    {
        // check that we can produce one element
        _inSemaphore.acquire();

        // enter critical section
        _producerConsumerMutex.lock();

        // produce the data
        _buffer[_appendIndex % kBufferSize] = datum;
        _appendIndex++;

        // exit critical section
        _producerConsumerMutex.unlock();

        // tell that we produced one element
        _outSemaphore.release();
    }

    uint32_t extract(void)
    {
        // check that we can consume one element
        _outSemaphore.acquire();

        // enter critical section
        _producerConsumerMutex.lock();

        // consumer one element        
        uint32_t datum = _buffer[_extractIndex % kBufferSize];
        _extractIndex++;
        
        // exit critical section
        _producerConsumerMutex.unlock();

        // tell that we consumed one element
        _inSemaphore.release();

        return datum;
    }

private:
    static constexpr uint8_t kBufferSize = 10;

    uint32_t _buffer[kBufferSize] = {0};
    uint32_t _appendIndex = 0;
    uint32_t _extractIndex = 0;
    Mutex _producerConsumerMutex;
    Semaphore _outSemaphore {0};
    Semaphore _inSemaphore {kBufferSize - 1};
};