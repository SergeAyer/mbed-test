#include "mbed.h"

#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

#include "buffer.hpp"
#include "producer.hpp"
#include "consumer.hpp"

using namespace utest::v1;

/** Test production and consumption of a given number of values
 *
 * Given a buffer, create a producer and consumer thread and make sure that
 * both threads return after a given number of values have been produced and consumed-
 */
static void check_producer_consumer()
{
    // create a buffer
    Buffer buffer;
    // nbr of values to be produced/consumed for the test
    static constexpr uint32_t nbrOfValues = 30;
    // create a producer and a consumer
    static constexpr std::chrono::microseconds kProduceWaitTime = 500000us;
    Producer producer(buffer, kProduceWaitTime, nbrOfValues);
    static constexpr std::chrono::microseconds kConsumeWaitTime = 500000us;
    Consumer consumer(buffer, kConsumeWaitTime, nbrOfValues);
    // start both
    producer.start();
    consumer.start();

    // wait for threads to terminate
    producer.wait();
    consumer.wait();
}

utest::v1::status_t greentea_setup(const size_t number_of_cases)
{
    // Here, we specify the timeout (60s) and the host test (a built-in host test or the name of our Python file)
    GREENTEA_SETUP(60, "default_auto");

    return greentea_test_setup_handler(number_of_cases);
}

// List of test cases in this file
Case cases[] = {
    Case("producer consumer test", check_producer_consumer)
};

Specification specification(greentea_setup, cases);

int main()
{
    return !Harness::run(specification);
}
