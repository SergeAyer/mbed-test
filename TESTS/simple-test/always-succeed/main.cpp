#include "mbed.h"
#include "utest/utest.h"
#include "unity/unity.h"
#include "greentea-client/test_env.h"

using namespace utest::v1;

// test handler function
static control_t always_succeed(const size_t call_count) {
  // this is the always succeed test
  TEST_ASSERT_EQUAL(4, 2 * 2);

  // execute the test only once and move to the next one, without waiting
  return CaseNext;
}

utest::v1::status_t greentea_setup(const size_t number_of_cases) {
  // Here, we specify the timeout (60s) and the host test (a built-in host test or the name of our Python file)
  GREENTEA_SETUP(60, "default_auto");

  return greentea_test_setup_handler(number_of_cases);
}

// List of test cases in this file
Case cases[] = {
  Case("always succeed test", always_succeed)
};

Specification specification(greentea_setup, cases);

int main() {
  return !Harness::run(specification);
}
