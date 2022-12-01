
#pragma once

class Buffer {
public:
    Buffer()
    {
    }

    void append(int datum)
    {
        // check that we can produce one element
        _inSemaphore.acquire();

        // enter critical section
        _producerConsumerMutex.lock();

        // produce the data
        _buffer[_index] = datum;
        _index++;

        // exit critical section
        _producerConsumerMutex.unlock();

        // tell that we produced one element
        _outSemaphore.release();
    }

    int extract(void)
    {
        // check that we can consume one element
        _outSemaphore.acquire();

        // enter critical section
        _producerConsumerMutex.lock();

        // consumer one element
        _index--;
        int datum = _buffer[_index];

        // exit critical section
        _producerConsumerMutex.unlock();

        // tell that we consumed one element
        _inSemaphore.release();

        return datum;
    }

private:
    static constexpr int kBufferSize = 10;

    int _buffer[kBufferSize] = {0};
    int _index = 0;
    Mutex _producerConsumerMutex;
    Semaphore _outSemaphore {0};
    Semaphore _inSemaphore {kBufferSize - 1};
};