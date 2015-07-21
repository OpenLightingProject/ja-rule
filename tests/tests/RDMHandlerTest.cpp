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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/RDMCommandSerializer.h>
#include <ola/rdm/RDMEnums.h>
#include <ola/rdm/UID.h>

#include "rdm_buffer.h"
#include "rdm_handler.h"
#include "utils.h"
#include "Array.h"
#include "Matchers.h"
#include "TestHelpers.h"

using ::testing::Return;
using ::testing::StrictMock;
using ::testing::WithArgs;
using ::testing::_;
using ola::rdm::GetResponseFromData;
using ola::rdm::RDMGetRequest;
using ola::rdm::RDMRequest;
using ola::rdm::RDMResponse;
using ola::rdm::RDMSetRequest;
using ola::rdm::UID;
using std::unique_ptr;

MATCHER_P(IOVecResponseIs, expected_response, "") {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*expected_response, &data));
  EXPECT_THAT(arg, PayloadIs(data.data(), data.size()));
  return true;
}

namespace {

void CallRDMHandler(const ola::rdm::RDMRequest *request) {
  ola::io::ByteString data;
  data.push_back(RDM_START_CODE);
  EXPECT_TRUE(ola::rdm::RDMCommandSerializer::Pack(*request, &data));
  RDMHandler_HandleRequest(AsHeader(data.data()), request->ParamData());
}

class MockModel {
 public:
  MOCK_METHOD0(Activate, void());
  MOCK_METHOD0(Deactivate, void());
  MOCK_METHOD3(Ioctl,
               int(ModelIoctl command, uint8_t *data, unsigned int length));
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

int IoctlFirst(ModelIoctl command, uint8_t *data, unsigned int length) {
  if (g_first_mock) {
    return g_first_mock->Ioctl(command, data, length);
  }
  return 0;
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

int IoctlSecond(ModelIoctl command, uint8_t *data, unsigned int length) {
  if (g_second_mock) {
    return g_second_mock->Ioctl(command, data, length);
  }
  return 0;
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
  RDMHandlerTest()
     : m_controller_uid(0x7a70, 0x00000000),
       m_our_uid(TEST_UID) {
  }

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

  void SetUID(uint8_t uid[UID_LENGTH]) {
    // Set to any non-0 value
    const uint8_t uninitialized_uid[UID_LENGTH] = {1, 2, 3, 4, 5, 6};
    memcpy(uid, uninitialized_uid, UID_LENGTH);
  }

 protected:
  UID m_controller_uid;
  UID m_our_uid;

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

  static const uint16_t MODEL_ONE;
  static const uint16_t MODEL_TWO;
  static const uint16_t MODEL_THREE;

  static const uint8_t SAMPLE_MESSAGE[];
  static const uint8_t NULL_UID[UID_LENGTH];
  static const uint8_t TEST_UID[UID_LENGTH];
};

const uint16_t RDMHandlerTest::MODEL_ONE = 1;
const uint16_t RDMHandlerTest::MODEL_TWO = 2;
const uint16_t RDMHandlerTest::MODEL_THREE = 3;

const uint8_t RDMHandlerTest::SAMPLE_MESSAGE[] = {
  0xcc, 0x01, 0x18, 0x7a, 0x70, 0x00, 0x00, 0x00, 0x00, 0x7a, 0x70, 0x12,
  0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x02, 0x00,
  0x03, 0xdf
};

const uint8_t RDMHandlerTest::NULL_UID[] = {0, 0, 0, 0, 0, 0};
const uint8_t RDMHandlerTest::TEST_UID[] = {0x7a, 0x70, 0, 0, 0, 1};

const ModelEntry RDMHandlerTest::FIRST_MODEL {
  MODEL_ONE,
  ActivateFirst,
  DeactivateFirst,
  IoctlFirst,
  RequestFirst,
  TasksFirst
};

const ModelEntry RDMHandlerTest::SECOND_MODEL {
  MODEL_TWO,
  ActivateSecond,
  DeactivateSecond,
  IoctlSecond,
  RequestSecond,
  TasksSecond
};

TEST_F(RDMHandlerTest, testDispatching) {
  RDMHandlerSettings settings = {
    .default_model = NULL_MODEL_ID,
    .send_callback = nullptr
  };
  uint8_t uid[UID_LENGTH];
  SetUID(uid);

  RDMHandler_Initialize(&settings);

  // No calls
  RDMHandler_GetUID(uid);
  EXPECT_THAT(uid, MatchesUID(NULL_UID));
  RDMHandler_HandleRequest(reinterpret_cast<const RDMHeader*>(SAMPLE_MESSAGE),
                           nullptr);
  RDMHandler_Tasks();

  EXPECT_TRUE(RDMHandler_AddModel(&FIRST_MODEL));
  EXPECT_TRUE(RDMHandler_AddModel(&SECOND_MODEL));
  EXPECT_FALSE(RDMHandler_AddModel(&SECOND_MODEL));

  // Still no calls
  SetUID(uid);
  RDMHandler_GetUID(uid);
  EXPECT_THAT(uid, MatchesUID(NULL_UID));
  RDMHandler_HandleRequest(reinterpret_cast<const RDMHeader*>(SAMPLE_MESSAGE),
                           nullptr);
  RDMHandler_Tasks();

  // Switch active model
  testing::InSequence seq;
  EXPECT_CALL(m_first_model, Activate()).Times(1);
  EXPECT_CALL(m_first_model, Ioctl(IOCTL_GET_UID, _, UID_LENGTH))
    .WillOnce(WithArgs<1>(CopyUID(TEST_UID)));
  EXPECT_CALL(m_first_model, Request(_, nullptr)).Times(1);
  EXPECT_CALL(m_first_model, Tasks()).Times(1);

  EXPECT_TRUE(RDMHandler_SetActiveModel(MODEL_ONE));

  SetUID(uid);
  RDMHandler_GetUID(uid);
  EXPECT_THAT(uid, MatchesUID(TEST_UID));

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
  EXPECT_TRUE(RDMHandler_SetActiveModel(NULL_MODEL_ID));
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

TEST_F(RDMHandlerTest, testGetSetModelId) {
  RDMHandlerSettings settings = {
    .default_model = MODEL_ONE,
    .send_callback = SendResponse
  };
  RDMHandler_Initialize(&settings);

  testing::InSequence seq;
  EXPECT_CALL(m_first_model, Activate()).Times(1);
  EXPECT_CALL(m_first_model, Ioctl(IOCTL_GET_UID, _, UID_LENGTH))
    .WillRepeatedly(WithArgs<1>(CopyUID(TEST_UID)));

  EXPECT_TRUE(RDMHandler_AddModel(&FIRST_MODEL));
  EXPECT_TRUE(RDMHandler_AddModel(&SECOND_MODEL));

  unique_ptr<RDMRequest> get_request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DEVICE_MODEL,
      nullptr, 0));

  uint16_t model_id = htons(MODEL_ONE);
  unique_ptr<RDMResponse> get_response(GetResponseFromData(
        get_request.get(),
        reinterpret_cast<const uint8_t*>(&model_id), sizeof(model_id)));

  EXPECT_CALL(m_sender_mock, SendResponse(true, _, 1))
      .With(testing::Args<1, 2>(IOVecResponseIs(get_response.get())));

  CallRDMHandler(get_request.get());

  // Now try a set
  uint16_t new_model_id = htons(MODEL_TWO);
  unique_ptr<RDMRequest> set_request(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DEVICE_MODEL,
      reinterpret_cast<const uint8_t*>(&new_model_id),
      sizeof(new_model_id)));
  unique_ptr<RDMResponse> set_response(GetResponseFromData(set_request.get()));

  EXPECT_CALL(m_first_model, Ioctl(IOCTL_GET_UID, _, UID_LENGTH))
    .WillRepeatedly(WithArgs<1>(CopyUID(TEST_UID)));
  EXPECT_CALL(m_first_model, Deactivate()).Times(1);
  EXPECT_CALL(m_second_model, Activate()).Times(1);
  EXPECT_CALL(m_sender_mock, SendResponse(true, _, 1))
      .With(testing::Args<1, 2>(IOVecResponseIs(set_response.get())));

  CallRDMHandler(set_request.get());
  EXPECT_EQ(MODEL_TWO, RDMHandler_ActiveModel());

  // Perform another get
  model_id = htons(MODEL_TWO);
  unique_ptr<RDMResponse> second_get_response(GetResponseFromData(
        get_request.get(),
        reinterpret_cast<const uint8_t*>(&model_id), sizeof(model_id)));

  EXPECT_CALL(m_second_model, Ioctl(IOCTL_GET_UID, _, UID_LENGTH))
    .WillRepeatedly(WithArgs<1>(CopyUID(TEST_UID)));
  EXPECT_CALL(m_sender_mock, SendResponse(true, _, 1))
      .With(testing::Args<1, 2>(IOVecResponseIs(second_get_response.get())));

  CallRDMHandler(get_request.get());

  // Now try a set for an invalid model
  new_model_id = MODEL_THREE;
  unique_ptr<RDMRequest> second_set_request(new RDMSetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DEVICE_MODEL,
      reinterpret_cast<const uint8_t*>(&new_model_id),
      sizeof(new_model_id)));
  unique_ptr<RDMResponse> nack_response(
      ola::rdm::NackWithReason(second_set_request.get(),
                               ola::rdm::NR_DATA_OUT_OF_RANGE));

  EXPECT_CALL(m_second_model, Ioctl(IOCTL_GET_UID, _, UID_LENGTH))
    .WillRepeatedly(WithArgs<1>(CopyUID(TEST_UID)));
  EXPECT_CALL(m_sender_mock, SendResponse(true, _, 1))
      .With(testing::Args<1, 2>(IOVecResponseIs(nack_response.get())));

  CallRDMHandler(second_set_request.get());
  EXPECT_EQ(MODEL_TWO, RDMHandler_ActiveModel());
}

TEST_F(RDMHandlerTest, testGetModelList) {
  RDMHandlerSettings settings = {
    .default_model = MODEL_ONE,
    .send_callback = SendResponse
  };
  RDMHandler_Initialize(&settings);

  testing::InSequence seq;
  EXPECT_CALL(m_first_model, Activate()).Times(1);
  EXPECT_CALL(m_first_model, Ioctl(IOCTL_GET_UID, _, UID_LENGTH))
    .WillRepeatedly(WithArgs<1>(CopyUID(TEST_UID)));

  EXPECT_TRUE(RDMHandler_AddModel(&FIRST_MODEL));
  EXPECT_TRUE(RDMHandler_AddModel(&SECOND_MODEL));

  unique_ptr<RDMRequest> get_request(new RDMGetRequest(
      m_controller_uid, m_our_uid, 0, 0, 0, PID_DEVICE_MODEL_LIST,
      nullptr, 0));

  uint8_t model_list[] = {
    0x00, 0x01,
    0x00, 0x02
  };
  unique_ptr<RDMResponse> get_response(GetResponseFromData(
        get_request.get(), model_list, arraysize(model_list)));

  EXPECT_CALL(m_sender_mock, SendResponse(true, _, 1))
      .With(testing::Args<1, 2>(IOVecResponseIs(get_response.get())));

  CallRDMHandler(get_request.get());
}
