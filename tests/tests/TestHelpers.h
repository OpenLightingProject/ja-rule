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
 * TestHelpers.h
 * Test helper functions.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/RDMCommandSerializer.h>

#include "constants.h"
#include "rdm.h"

using ola::rdm::RDMRequest;

/*
 * @brief Check an RDM response matches the expected value.
 */
MATCHER_P(ResponseIs, expected_response, "") {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*expected_response, &data));

  return MemoryCompare(reinterpret_cast<const uint8_t*>(std::get<0>(arg)),
                       std::get<1>(arg), data.data(), data.size(),
                       result_listener);
}

/*
 * @brief Cast a pointer to an RDMHeader.
 */
const RDMHeader *AsHeader(const uint8_t *data) {
  return reinterpret_cast<const RDMHeader*>(data);
}

template<typename Func>
int InvokeHandler(Func function, const ola::rdm::RDMRequest *request) {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));
  return function(AsHeader(data.data()), request->ParamData());
}

template<typename Func>
int InvokeMuteHandler(Func function, const ola::rdm::RDMRequest *request) {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));
  return function(AsHeader(data.data()));
}
