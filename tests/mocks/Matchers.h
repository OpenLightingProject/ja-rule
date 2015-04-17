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
 * Matchers.h
 * Useful matchers.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_MATCHERS_H_
#define TESTS_MOCKS_MATCHERS_H_

#include <gmock/gmock.h>
#include <stdint.h>
#include "iovec.h"

/**
 * @defgroup testing Testing Utilities
 * @brief Helpers for unit testing.
 *
 * @addtogroup testing
 * @{
 * @file Matchers.h
 * @brief Mock Matchers
 */

/**
 * @brief An IOVec and it's associated length.
 */
typedef ::testing::tuple<const IOVec*, unsigned int> IOVecTuple;

/**
 * @brief Check that a {pointer, length} tuple matches the expected data.
 * @param expected_data A pointer to the expected data.
 * @param expected_size The size of the expected data.
 *
 * @examplepara
 * ~~~~~~~~~~~~~~~~~~~~~
 *   EXPECT_CALL(mock, Send(...))
 *       .With(Args<1, 2>(DataIs(ptr, length));
 * ~~~~~~~~~~~~~~~~~~~~~
 */
testing::Matcher< ::testing::tuple<const void*, unsigned int> > DataIs(
    const uint8_t* expected_data,
    unsigned int expected_size);

/**
 * @brief Check that an IOVec matches the expected data.
 * @param expected_data A pointer to the expected payload data.
 * @param expected_size The size of the expected payload data.
 *
 * @examplepara
 * ~~~~~~~~~~~~~~~~~~~~~
 *   EXPECT_CALL(mock, Send(...))
 *       .With(Args<1, 2>(PayloadIs(expected_data, arraysize(expected_data)));
 * ~~~~~~~~~~~~~~~~~~~~~
 */
testing::Matcher<IOVecTuple> PayloadIs(const uint8_t* expected_data,
                                       unsigned int expected_size);

/**
 * @brief Check that an IOVec contains no data.
 *
 * @examplepara
 * ~~~~~~~~~~~~~~~~~~~~~
 *   EXPECT_CALL(mock, Send(...))
 *       .With(Args<1, 2>(EmptyPayload());
 * ~~~~~~~~~~~~~~~~~~~~~
 */
testing::Matcher<IOVecTuple> EmptyPayload();

/**
 * @}
 */
#endif  // TESTS_MOCKS_MATCHERS_H_
