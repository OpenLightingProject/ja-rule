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
 * RDMHandlerTest.cpp
 * Tests for the RDMHandler code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "rdm_handler.h"

using ::testing::StrictMock;
using ::testing::Return;
using ::testing::_;

namespace {

class MockModel {
 public:
  MOCK_METHOD0(Activate, void());
  MOCK_METHOD0(Deactivate, void());
  MOCK_METHOD2(Request,
               int(const RDMHeader *header, const uint8_t *param_data));
  MOCK_METHOD0(Tasks, void());
};

MockModel *g_first_mock = nullptr;
MockModel *g_second_mock = nullptr;

void ActivateFirst() {
  if (g_first_mock) {
    g_first_mock->Activate();
  }
}

void DeactivateFirst() {
  if (g_first_mock) {
    g_first_mock->Deactivate();
  }
}

int RequestFirst(const RDMHeader *header, const uint8_t *param_data) {
  if (g_first_mock) {
    return g_first_mock->Request(header, param_data);
  }
  return 0;
}

void TasksFirst() {
  if (g_first_mock) {
    g_first_mock->Tasks();
  }
}

void ActivateSecond() {
  if (g_second_mock) {
    g_second_mock->Activate();
  }
}

void DeactivateSecond() {
  if (g_second_mock) {
    g_second_mock->Deactivate();
  }
}

int RequestSecond(const RDMHeader *header, const uint8_t *param_data) {
  if (g_second_mock) {
    return g_second_mock->Request(header, param_data);
  }
  return 0;
}

void TasksSecond() {
  if (g_second_mock) {
    g_second_mock->Tasks();
  }
}

class MockSender {
 public:
  MOCK_METHOD3(SendResponse, void(bool include_break,
                                  const IOVec* data,
                                  unsigned int iov_count));
};

MockSender *g_sender = nullptr;

void SendResponse(bool include_break,
                  const IOVec* data,
                  unsigned int iov_count) {
  if (g_sender) {
    g_sender->SendResponse(include_break, data, iov_count);
  }
}

}  // namespace

class RDMHandlerTest : public testing::Test {
 public:
  void SetUp() {
    g_first_mock = &m_first_model;
    g_second_mock = &m_second_model;
    g_sender = &m_sender_mock;
  }

  void TearDown() {
    g_first_mock = nullptr;
    g_second_mock = nullptr;
    g_sender = nullptr;
  }

 protected:
  StrictMock<MockModel> m_first_model;
  StrictMock<MockModel> m_second_model;
  StrictMock<MockSender> m_sender_mock;


  static const uint8_t OUR_UID[];
  static const uint8_t VENDORCAST_UID[];
  static const uint8_t BROADCAST_UID[];
  static const uint8_t OTHER_VENDORCAST_UID[];
  static const uint8_t OTHER_UID[];

  static const ModelEntry FIRST_MODEL;
  static const ModelEntry SECOND_MODEL;

  static const uint16_t MODEL_ONE = 1;
  static const uint16_t MODEL_TWO = 2;
  static const uint16_t MODEL_THREE = 3;

  static const uint8_t SAMPLE_MESSAGE[];
};

const uint8_t RDMHandlerTest::SAMPLE_MESSAGE[] = {
  0xcc, 0x01, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
  0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00,
  0x03, 0xdf
};

const ModelEntry RDMHandlerTest::FIRST_MODEL {
  .model_id = MODEL_ONE,
  .activate_fn = ActivateFirst,
  .deactivate_fn = DeactivateFirst,
  .request_fn = RequestFirst,
  .tasks_fn = TasksFirst
};

const ModelEntry RDMHandlerTest::SECOND_MODEL {
  .model_id = MODEL_TWO,
  .activate_fn = ActivateSecond,
  .deactivate_fn = DeactivateSecond,
  .request_fn = RequestSecond,
  .tasks_fn = TasksSecond
};

TEST_F(RDMHandlerTest, testDispatching) {
  RDMHandlerSettings settings = {
    .default_model = NULL_MODEL,
    .send_callback = nullptr
  };

  RDMHandler_Initialize(&settings);

  // No calls
  RDMHandler_HandleRequest(reinterpret_cast<const RDMHeader*>(SAMPLE_MESSAGE),
                           nullptr);
  RDMHandler_Tasks();

  EXPECT_TRUE(RDMHandler_AddModel(&FIRST_MODEL));
  EXPECT_TRUE(RDMHandler_AddModel(&SECOND_MODEL));
  EXPECT_FALSE(RDMHandler_AddModel(&SECOND_MODEL));

  // Still no calls
  RDMHandler_HandleRequest(reinterpret_cast<const RDMHeader*>(SAMPLE_MESSAGE),
                           nullptr);
  RDMHandler_Tasks();

  // Switch active model
  testing::InSequence seq;
  EXPECT_CALL(m_first_model, Activate()).Times(1);
  EXPECT_CALL(m_first_model, Request(_, nullptr)).Times(1);
  EXPECT_CALL(m_first_model, Tasks()).Times(1);

  EXPECT_TRUE(RDMHandler_SetActiveModel(MODEL_ONE));
  RDMHandler_HandleRequest(reinterpret_cast<const RDMHeader*>(SAMPLE_MESSAGE),
                           nullptr);
  RDMHandler_Tasks();

  // Switch to model 2
  EXPECT_CALL(m_first_model, Deactivate()).Times(1);
  EXPECT_CALL(m_second_model, Activate()).Times(1);

  EXPECT_TRUE(RDMHandler_SetActiveModel(MODEL_TWO));
  EXPECT_TRUE(RDMHandler_SetActiveModel(MODEL_TWO));
  EXPECT_CALL(m_second_model, Request(_, nullptr)).Times(1);
  EXPECT_CALL(m_second_model, Tasks()).Times(1);

  RDMHandler_HandleRequest(reinterpret_cast<const RDMHeader*>(SAMPLE_MESSAGE),
                           nullptr);
  RDMHandler_Tasks();

  // Try an invalid model
  EXPECT_FALSE(RDMHandler_SetActiveModel(MODEL_THREE));

  // Switch back to the nullptr model
  EXPECT_CALL(m_second_model, Deactivate()).Times(1);
  EXPECT_TRUE(RDMHandler_SetActiveModel(NULL_MODEL));
}

TEST_F(RDMHandlerTest, testSendResponse) {
  RDMHandlerSettings settings = {
    .default_model = MODEL_ONE,
    .send_callback = SendResponse
  };
  RDMHandler_Initialize(&settings);

  testing::InSequence seq;
  EXPECT_CALL(m_first_model, Activate()).Times(1);
  EXPECT_CALL(m_first_model, Request(_, nullptr))
    .WillOnce(Return(-24));
  EXPECT_CALL(m_sender_mock, SendResponse(false, _, 1)).Times(1);
  EXPECT_CALL(m_first_model, Request(_, nullptr)).WillOnce(Return(26));
  EXPECT_CALL(m_sender_mock, SendResponse(true, _, 1)).Times(1);

  EXPECT_TRUE(RDMHandler_AddModel(&FIRST_MODEL));
  RDMHandler_HandleRequest(reinterpret_cast<const RDMHeader*>(SAMPLE_MESSAGE),
                           nullptr);
  RDMHandler_HandleRequest(reinterpret_cast<const RDMHeader*>(SAMPLE_MESSAGE),
                           nullptr);
}
