/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * CoarseTimerTest.cpp
 * Tests for the CoarseTimer code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include "coarse_timer.h"

class CoarseTimerTest : public ::testing::TestWithParam<uint32_t> {
 public:
  void SetUp() {
    CoarseTimer_Settings timer_settings = {
      .timer_id = TMR_ID_2,
      .interrupt_source = INT_SOURCE_TIMER_2
    };
    CoarseTimer_Initialize(&timer_settings);
  }
};

TEST_P(CoarseTimerTest, TimerWorks) {
  CoarseTimer_SetCounter(GetParam());
  EXPECT_EQ(GetParam(), CoarseTimer_GetTime());

  uint32_t start = CoarseTimer_GetTime();
  EXPECT_EQ(0, CoarseTimer_ElapsedTime(start));
  EXPECT_TRUE(CoarseTimer_HasElapsed(start, 0));

  EXPECT_FALSE(CoarseTimer_HasElapsed(start, 1));
  EXPECT_FALSE(CoarseTimer_HasElapsed(start, 2));
  EXPECT_FALSE(CoarseTimer_HasElapsed(start, 10));

  // First tick.
  CoarseTimer_TimerEvent();

  EXPECT_EQ(1, CoarseTimer_ElapsedTime(start));
  EXPECT_TRUE(CoarseTimer_HasElapsed(start, 0));
  EXPECT_TRUE(CoarseTimer_HasElapsed(start, 1));
  EXPECT_FALSE(CoarseTimer_HasElapsed(start, 2));
  EXPECT_FALSE(CoarseTimer_HasElapsed(start, 10));

  // Cycle until 100 ticks (10ms) have elapsed.
  unsigned int timer_events = 0;
  while (!CoarseTimer_HasElapsed(start, 100)) {
    timer_events++;
    CoarseTimer_TimerEvent();
  }
  EXPECT_EQ(100, CoarseTimer_ElapsedTime(start));
  EXPECT_EQ(99, timer_events);
}

INSTANTIATE_TEST_CASE_P(InstantiationName,
                        CoarseTimerTest,
                        ::testing::Values(0, 1, 52, 0xfffffffe, 0xffffffff));
