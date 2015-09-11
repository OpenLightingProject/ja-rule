Unit Tests  {#testing}
========================

[TOC]

# Overview {#testing-overview}

In order to maintain software quality and speed development the code is unit
tested. Even though the project is written in C, we use
[Mock Objects](https://en.wikipedia.org/wiki/Mock_object) to
intercept function calls and set expectations.

# Initial Setup {#testing-setup}

The unit tests use the [gmock](https://code.google.com/p/googlemock/) and
[gtest](https://code.google.com/p/googletest/) frameworks. To
install GMock, run ./install-gmock.sh in the top level project directory.
This will download & install gmock / gtest within the local project
directory.

@warning gmock / gtest should not be installed system-wide, see
  https://code.google.com/p/googletest/wiki/FAQ .

# Mocks {#testing-mocks}

Two sets of Mocks are provided:

## Harmony Mocks {#testing-mocks-harony}

Any Harmony modules that we use have their API calls proxied through to a
mock object. This enables us to stub out the Harmony API for testing. The
Harmony mocks consist of a set of headers (found in harmony/include) and the
mock harmony objects (in harmony/mocks).

The following is a simple example of how to use the Harmony mocks.

timer.c
~~~~~~~~~~~~~~~~~~~~~
// Code to test
#include "peripheral/tmr/plib_tmr.h"

void StartTimer(uint16_t counter) {
  PLIB_TMR_Counter16BitClear(TMR_ID_2);
  PLIB_TMR_Period16BitSet(TMR_ID_2, counter);
  PLIB_TMR_Start(TMR_ID_2);
}
~~~~~~~~~~~~~~~~~~~~~

TimerTest.cpp
~~~~~~~~~~~~~~~~~~~~~
#include <gtest/gtest.h>
#include "plib_tmr_mock.h"

class TimerTest : public testing::Test {
 public:
  void SetUp() {
    PLIB_TMR_SetMock(&m_mock);
  }

  void TearDown() {
    PLIB_TMR_SetMock(NULL);
  }
  MockPeripheralTimer m_mock;
};

TEST(TimerTest, testZeroCounter) {
  EXPECT_CALL(mock, Counter16BitClear(TMR_ID_2));
  EXPECT_CALL(mock, Period16BitSet(TMR_ID_2, 10));
  EXPECT_CALL(mock, Start(TMR_ID_2));
  StartTimer(10);
}
~~~~~~~~~~~~~~~~~~~~~

# Module Mocks {#testing-mocks-modules}

When one Ja Rule module depends on another, it's often useful to be able to
mock out the dependency. Ja Rule module mocks can be found in the
mocks/ directory. They work in the same way as the Harmony mocks; calls to
the module's functions are proxied to a Mock object.

There is also a set of Matchers that can be used to check
that binary data passed to functions matches what is expected.
