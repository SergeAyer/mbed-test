
#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"
#include <chrono>
#include <cstdint>
#include <ratio>

using namespace utest::v1;

// constants used in test functions
static constexpr std::chrono::milliseconds TEST_TIMEOUT = 50ms;
static constexpr std::chrono::milliseconds DELTA = 1ms;

// test handler functions

/** Test get empty without timeout
 *
 * Given an empty queue with one slot for uint32_t data.
 * When a thread tries to get one messages without timeout
*  then the try_get() method returns false.
 */
static void get_empty_no_timeout() {
  // create a simple queue
  Queue<uint32_t, 1> q;

  // try to get an element from the empty queue
  uint32_t* v = nullptr;
  bool res = q.try_get(&v);
  // should not succeed
  TEST_ASSERT_FALSE(res);
}

/** Test put full timeout
 *
 * Given a queue with two slots for uint32_t data.
 * When a thread tries to insert two messages with @ TEST_TIMEOUT timeout,
 * then the first try_put_for() operation succeeds and 
 * the second try_put_for() operation fails with the expected timeout.
 */
static void put_full_timeout() {
  // create a queue with 2 elements
  Queue<uint32_t, 2> q;

  // insert one element
  uint32_t msg = 0;
  bool res = q.try_put_for(TEST_TIMEOUT, &msg);
  // should succeed
  TEST_ASSERT_TRUE(res);

  // insert a second element
  res = q.try_put_for(TEST_TIMEOUT, &msg);
  // should succeed
  TEST_ASSERT_TRUE(res);

  // try to get an element with a given timeout
  Timer timer;
  timer.start();

  res = q.try_put_for(TEST_TIMEOUT, &msg);
  // should fail
  TEST_ASSERT_FALSE(res);
  // elapsed time should match the timeout value
  TEST_ASSERT_INT_WITHIN(std::chrono::microseconds(DELTA).count(), 
                         std::chrono::microseconds(TEST_TIMEOUT).count(), 
                         timer.elapsed_time().count());
}

/** Test put full with forever timeout.
 *
 * Given the main thread and a consumer thread- Given a queue with one slot for uint32_t data.
 * When the main thread puts a message to the queue and tries to put second 
 * one with @a Kernel::wait_for_u32_forever timeout
 * Then thread waits for a slot to become empty in the queue
 * When the consumer thread takes one message out of the queue
 * Then the main thread successfully inserts message into the queue.
 */

// global variable used for checking messages between consumer and producer
uint32_t msg = 0;

// function used by a separate thread for putting one message to a queue
void thread_put_uint_msg(Queue<uint32_t, 1>* q) {
  // wait before producing
  ThisThread::sleep_for(TEST_TIMEOUT);
  // try to consume one element with infinite timeout
  msg = 2;
  bool res = q->try_put_for(Kernel::wait_for_u32_forever, &msg);
  // should succeed (since one element was consumed)
  TEST_ASSERT_TRUE(res);
}

void test_get_empty_waitforever() {
  // create a queue with one element
  Queue<uint32_t, 1> q;
  
  // The consumer thread will run as the main thread
  // launch a separate thread as a producer of data (with limited stack size)
  static constexpr uint32_t THREAD_STACK_SIZE = 512;
  Thread t(osPriorityNormal, THREAD_STACK_SIZE);
  t.start(callback(thread_put_uint_msg, &q));

  // try to consume one element
  uint32_t* value = nullptr;
  Timer timer;
  timer.start();
  bool res = q.try_get_for(Kernel::wait_for_u32_forever, &value);
  // should succeed since one element was produced
  TEST_ASSERT_TRUE(res);
  // the consumed value should match the produced one
  TEST_ASSERT_EQUAL(&msg, value);
  // elapsed time should match the timeout value
  TEST_ASSERT_INT_WITHIN(std::chrono::microseconds(DELTA).count(), 
                         std::chrono::microseconds(TEST_TIMEOUT).count(), 
                         timer.elapsed_time().count());
  
  // wait for the producer thread to exit
  t.join();
}

// function used by a separate thread for getting one message from a queue
void thread_get_uint_msg(Queue<uint32_t, 1>* q) {
  // wait before consuming
  ThisThread::sleep_for(TEST_TIMEOUT);
  // try to consume one element with infinite timeout
  uint32_t* value = nullptr;
  bool res = q->try_get_for(Kernel::wait_for_u32_forever, &value);
  // should succeed (since one element was produced)
  TEST_ASSERT_TRUE(res);
  // the value consumed should match the value produced
  TEST_ASSERT_EQUAL(&msg, value);
}

void test_put_full_waitforever() {
  // create a queue with one element
  Queue<uint32_t, 1> q;
  
  // The producer thread will run as the main thread
  // launch a separate thread as a consumer of data (with limited stack size)
  static constexpr uint32_t THREAD_STACK_SIZE = 512;
  Thread t(osPriorityNormal, THREAD_STACK_SIZE);
  t.start(callback(thread_get_uint_msg, &q));

  // producing one element should succeed
  msg = 1;
  bool res = q.try_put(&msg);
  TEST_ASSERT_TRUE(res);

  // try to produce a second element  
  Timer timer;
  timer.start();
  res = q.try_put_for(Kernel::wait_for_u32_forever, &msg);
  // should succeed since the element produced above was consumed
  TEST_ASSERT_TRUE(res);
   // elapsed time should match the timeout value
  TEST_ASSERT_INT_WITHIN(std::chrono::microseconds(DELTA).count(), 
                         std::chrono::microseconds(TEST_TIMEOUT).count(), 
                         timer.elapsed_time().count());
      //TEST_ASSERT_DURATION_WITHIN(TEST_TIMEOUT / 10, TEST_TIMEOUT, timer.elapsed_time());

  // 
  // wait for the consumer thread to exit
  t.join();
}

utest::v1::status_t greentea_setup(const size_t number_of_cases) {
  // Here, we specify the timeout (60s) and the host test (a built-in host test or the name of our Python file)
  GREENTEA_SETUP(60, "default_auto");

  return greentea_test_setup_handler(number_of_cases);
}

// List of test cases in this file
Case cases[] = {
  Case("try to get an element from empty queue without timeout", get_empty_no_timeout),
  Case("try to put an element to full queue with timeout", put_full_timeout),
  Case("try to get elements from empty queue forever", test_get_empty_waitforever),
  Case("try to put elements to full queue forever", test_put_full_waitforever)
};

Specification specification(greentea_setup, cases);

int main() {
  return !Harness::run(specification);
}
