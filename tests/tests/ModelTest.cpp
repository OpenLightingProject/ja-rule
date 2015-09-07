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
 * ModelTest.cpp
 * Common base test for the Models.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include <ola/rdm/UID.h>
#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/RDMEnums.h>
#include <ola/rdm/RDMCommandSerializer.h>
#include <ola/network/NetworkUtils.h>
#include <string.h>
#include <memory>

#include "constants.h"
#include "rdm.h"
#include "TestHelpers.h"
#include "ModelTest.h"

using ola::rdm::UID;
using ola::rdm::RDMGetRequest;
using ola::rdm::RDMRequest;
using ola::rdm::RDMSetRequest;
using std::unique_ptr;

const uint8_t ModelTest::TEST_UID[] = {
  0x7a, 0x70, 0x12, 0x34, 0x56, 0x78
};

ModelTest::ModelTest(const ModelEntry *model)
    : m_controller_uid(0x7a70, 0x00000000),
      m_our_uid(TEST_UID),
      m_model(model) {
}

unique_ptr<RDMRequest> ModelTest::BuildGetRequest(
    uint16_t pid,
    const uint8_t *param_data,
    unsigned int param_data_size) {
  return unique_ptr<RDMGetRequest>(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, pid, param_data,
      param_data_size));
}

unique_ptr<RDMRequest> ModelTest::BuildSetRequest(
    uint16_t pid,
    const uint8_t *param_data,
    unsigned int param_data_size) {
  return unique_ptr<RDMSetRequest>(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, pid, param_data,
      param_data_size));
}

int ModelTest::InvokeRDMHandler(const ola::rdm::RDMRequest *request) {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));

  return m_model->request_fn(
      AsHeader(data.data()), request->ParamData());
}
