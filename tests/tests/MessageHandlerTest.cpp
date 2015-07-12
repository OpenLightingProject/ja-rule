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
 * MessageHandlerTest.cpp
 * Tests for the MessageHandler code.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include "AppMock.h"
#include "Array.h"
#include "FlagsMock.h"
#include "LoggerMock.h"
#include "Matchers.h"
#include "TransceiverMock.h"
#include "TransportMock.h"
#include "constants.h"
#include "message_handler.h"

using ::testing::Args;
using ::testing::Return;
using ::testing::_;


// Tests for configuration messages.
// ----------------------------------------------------------------------------
struct ConfigurationTestArgs {
 public:
  ConfigurationTestArgs(Command get_command,
                        Command set_command,
                        uint16_t value)
    : get_command(get_command),
      set_command(set_command),
      value(value) {
  }

  Command get_command;
  Command set_command;
  uint16_t value;
};

class ConfigurationTest
    : public ::testing::TestWithParam<ConfigurationTestArgs> {
 public:
  void SetUp() {
    Transport_SetMock(&m_transport_mock);
    Transceiver_SetMock(&m_transceiver_mock);
    MessageHandler_Initialize(Transport_Send);
  }
  void TearDown() {
    Transceiver_SetMock(nullptr);
    Transport_SetMock(nullptr);
  }

  MockTransport m_transport_mock;
  MockTransceiver m_transceiver_mock;
  static const uint8_t kToken = 0;
};

TEST_P(ConfigurationTest, CheckOversizedGetRequest) {
  ConfigurationTestArgs args = GetParam();
  EXPECT_CALL(m_transport_mock,
              Send(kToken, args.get_command, RC_BAD_PARAM, _, 0))
      .WillOnce(Return(true));

  uint16_t payload = 0;
  Message large_message = {
    kToken, static_cast<uint16_t>(args.get_command),
    sizeof(payload),
    reinterpret_cast<uint8_t*>(&payload)
  };
  MessageHandler_HandleMessage(&large_message);
}

TEST_P(ConfigurationTest , CheckZeroLengthSetRequest) {
  ConfigurationTestArgs args = GetParam();
  EXPECT_CALL(m_transport_mock,
              Send(kToken, args.set_command, RC_BAD_PARAM, _, 0))
      .WillOnce(Return(true));

  Message small_message = {
    kToken, static_cast<uint16_t>(args.set_command), 0, NULL
  };
  MessageHandler_HandleMessage(&small_message);
}

TEST_P(ConfigurationTest, CheckOversizedSetRequest) {
  ConfigurationTestArgs args = GetParam();
  EXPECT_CALL(m_transport_mock,
              Send(kToken, args.set_command, RC_BAD_PARAM, _, 0))
      .WillOnce(Return(true));

  uint32_t payload = 0;
  Message large_message = {
    kToken, static_cast<uint16_t>(args.set_command),
    sizeof(payload),
    reinterpret_cast<uint8_t*>(&payload)
  };
  MessageHandler_HandleMessage(&large_message);
}

TEST_P(ConfigurationTest, CheckSetGet) {
  ConfigurationTestArgs args = GetParam();
  EXPECT_CALL(m_transport_mock,
              Send(kToken, args.set_command, RC_OK, _, 0))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken, args.get_command, RC_OK, _, 1))
      .With(Args<3, 4>(PayloadIs(reinterpret_cast<uint8_t*>(&args.value),
                                 sizeof(args.value))))
      .WillOnce(Return(true));

  switch (args.get_command) {
    case COMMAND_GET_BREAK_TIME:
      EXPECT_CALL(m_transceiver_mock, SetBreakTime(args.value))
          .WillOnce(Return(true));
      EXPECT_CALL(m_transceiver_mock, GetBreakTime())
          .WillOnce(Return(args.value));
      break;
    case COMMAND_GET_MARK_TIME:
      EXPECT_CALL(m_transceiver_mock, SetMarkTime(args.value))
          .WillOnce(Return(true));
      EXPECT_CALL(m_transceiver_mock, GetMarkTime())
          .WillOnce(Return(args.value));
      break;
    case COMMAND_GET_RDM_BROADCAST_TIMEOUT:
      EXPECT_CALL(m_transceiver_mock, SetRDMBroadcastTimeout(args.value))
          .WillOnce(Return(true));
      EXPECT_CALL(m_transceiver_mock, GetRDMBroadcastTimeout())
          .WillOnce(Return(args.value));
      break;
    case COMMAND_GET_RDM_RESPONSE_TIMEOUT:
      EXPECT_CALL(m_transceiver_mock, SetRDMResponseTimeout(args.value))
          .WillOnce(Return(true));
      EXPECT_CALL(m_transceiver_mock, GetRDMResponseTimeout())
          .WillOnce(Return(args.value));
      break;
    case COMMAND_GET_RDM_DUB_RESPONSE_LIMIT:
      EXPECT_CALL(m_transceiver_mock, SetRDMDUBResponseLimit(args.value))
          .WillOnce(Return(true));
      EXPECT_CALL(m_transceiver_mock, GetRDMDUBResponseLimit())
          .WillOnce(Return(args.value));
      break;
    case COMMAND_GET_RDM_RESPONDER_DELAY:
      EXPECT_CALL(m_transceiver_mock, SetRDMResponderDelay(args.value))
          .WillOnce(Return(true));
      EXPECT_CALL(m_transceiver_mock, GetRDMResponderDelay())
          .WillOnce(Return(args.value));
      break;
    case COMMAND_GET_RDM_RESPONDER_JITTER:
      EXPECT_CALL(m_transceiver_mock, SetRDMResponderJitter(args.value))
          .WillOnce(Return(true));
      EXPECT_CALL(m_transceiver_mock, GetRDMResponderJitter())
          .WillOnce(Return(args.value));
      break;
    default:
      {}
  }

  Message set_message = { kToken, static_cast<uint16_t>(args.set_command),
                          sizeof(args.value),
                          reinterpret_cast<uint8_t*>(&args.value) };
  MessageHandler_HandleMessage(&set_message);

  Message get_message = { kToken, static_cast<uint16_t>(args.get_command),
                          0, NULL};
  MessageHandler_HandleMessage(&get_message);
}

INSTANTIATE_TEST_CASE_P(
    SetCommand,
    ConfigurationTest,
    ::testing::Values(
      ConfigurationTestArgs(COMMAND_GET_BREAK_TIME, COMMAND_SET_BREAK_TIME, 88),
      ConfigurationTestArgs(COMMAND_GET_MARK_TIME, COMMAND_SET_MARK_TIME, 16),
      ConfigurationTestArgs(COMMAND_GET_RDM_BROADCAST_TIMEOUT,
                            COMMAND_SET_RDM_BROADCAST_TIMEOUT, 20),
      ConfigurationTestArgs(COMMAND_GET_RDM_RESPONSE_TIMEOUT,
                            COMMAND_SET_RDM_RESPONSE_TIMEOUT, 20),
      ConfigurationTestArgs(COMMAND_GET_RDM_DUB_RESPONSE_LIMIT,
                            COMMAND_SET_RDM_DUB_RESPONSE_LIMIT, 20000),
      ConfigurationTestArgs(COMMAND_GET_RDM_RESPONDER_DELAY,
                            COMMAND_SET_RDM_RESPONDER_DELAY, 2000),
      ConfigurationTestArgs(COMMAND_GET_RDM_RESPONDER_JITTER,
                            COMMAND_SET_RDM_RESPONDER_JITTER, 10)));

// Non-parametized tests.
// ----------------------------------------------------------------------------

class MessageHandlerTest : public testing::Test {
 public:
  void SetUp() {
    Transport_SetMock(&m_transport_mock);
    Transceiver_SetMock(&m_transceiver_mock);
    MessageHandler_Initialize(Transport_Send);
  }

  void TearDown() {
    Transceiver_SetMock(nullptr);
    Flags_SetMock(nullptr);
    Logger_SetMock(nullptr);
    Transport_SetMock(nullptr);
  }

  void SendEvent(uint8_t token, TransceiverOperation op,
                 TransceiverOperationResult result, const uint8_t *data,
                 unsigned int length) {
    TransceiverTiming timing;
    memset(reinterpret_cast<uint8_t*>(&timing), 0, sizeof(timing));
    TransceiverEvent event {
      .token = token,
      .op = op,
      .result = result,
      .data = data,
      .length = length,
      .timing = &timing
    };
    MessageHandler_TransceiverEvent(&event);
  }

 protected:
  MockTransport m_transport_mock;
  MockTransceiver m_transceiver_mock;

  static const uint8_t kToken = 0;
  static const uint8_t kEmptyDUBResponse[];
  static const uint8_t kEmptyRDMResponse[];
};

const uint8_t MessageHandlerTest::kEmptyDUBResponse[] = {0, 0, 0, 0};
const uint8_t MessageHandlerTest::kEmptyRDMResponse[] = {0, 0, 0, 0, 0, 0};

TEST_F(MessageHandlerTest, testEcho) {
  const uint8_t echo_payload[] = {1, 3, 4, 4};

  EXPECT_CALL(m_transport_mock, Send(kToken, ECHO, RC_OK, _, 1))
      .With(Args<3, 4>(PayloadIs(echo_payload, arraysize(echo_payload))))
      .WillOnce(Return(true));

  Message message = { kToken, ECHO, arraysize(echo_payload), &echo_payload[0] };
  MessageHandler_HandleMessage(&message);
}

TEST_F(MessageHandlerTest, testSetMode) {
  EXPECT_CALL(m_transport_mock, Send(kToken, COMMAND_SET_MODE, RC_OK, _, 0))
      .Times(2)
      .WillRepeatedly(Return(true));
  // TODO(simon): Add the transceiver expectaction here

  uint8_t request_payload = 0;
  Message message = { kToken, COMMAND_SET_MODE, sizeof(request_payload),
                      &request_payload };
  MessageHandler_HandleMessage(&message);

  request_payload = 1;
  MessageHandler_HandleMessage(&message);
}

TEST_F(MessageHandlerTest, testDMX) {
  const uint8_t dmx_data[] = {1, 3, 4, 4};

  testing::InSequence seq;
  EXPECT_CALL(m_transceiver_mock, QueueDMX(_, _, arraysize(dmx_data)))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transceiver_mock, QueueDMX(_, _, arraysize(dmx_data)))
      .WillOnce(Return(false));
  EXPECT_CALL(m_transport_mock, Send(kToken, TX_DMX, RC_BUFFER_FULL, NULL, 0))
      .WillOnce(Return(true));

  Message message = { kToken, TX_DMX, arraysize(dmx_data), &dmx_data[0] };
  MessageHandler_HandleMessage(&message);
  MessageHandler_HandleMessage(&message);
}

TEST_F(MessageHandlerTest, testLogger) {
  MockLogger logger_mock;
  Logger_SetMock(&logger_mock);

  EXPECT_CALL(logger_mock, SendResponse(kToken));

  Message message = { kToken, GET_LOG, 0, NULL };
  MessageHandler_HandleMessage(&message);
}

TEST_F(MessageHandlerTest, testFlags) {
  MockFlags flags_mock;
  Flags_SetMock(&flags_mock);

  EXPECT_CALL(flags_mock, SendResponse(kToken));

  Message message = { kToken, GET_FLAGS, 0, NULL };
  MessageHandler_HandleMessage(&message);
}

TEST_F(MessageHandlerTest, testReset) {
  MockApp app_mock;
  APP_SetMock(&app_mock);

  EXPECT_CALL(app_mock, Reset());
  EXPECT_CALL(m_transport_mock,
              Send(kToken, COMMAND_RESET_DEVICE, RC_OK, NULL, 0))
      .WillOnce(Return(true));

  Message message = { kToken, COMMAND_RESET_DEVICE, 0, NULL };
  MessageHandler_HandleMessage(&message);
}

TEST_F(MessageHandlerTest, testUnknownMessage) {
  EXPECT_CALL(m_transport_mock,
              Send(kToken, (Command) 0xff, RC_UNKNOWN, NULL, 0))
      .WillOnce(Return(true));

  Message message = { kToken, (Command) 0xff, 0, NULL };
  MessageHandler_HandleMessage(&message);
}

TEST_F(MessageHandlerTest, transceiverDMXEvent) {
  EXPECT_CALL(m_transport_mock, Send(kToken, TX_DMX, RC_OK, _, _))
      .With(Args<3, 4>(EmptyPayload()))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock, Send(kToken + 1, TX_DMX, RC_TX_ERROR, _, _))
      .With(Args<3, 4>(EmptyPayload()))
      .WillOnce(Return(true));

  SendEvent(kToken, T_OP_TX_ONLY, T_RESULT_TX_OK, NULL, 0);
  SendEvent(kToken + 1, T_OP_TX_ONLY, T_RESULT_TX_ERROR, NULL, 0);
}

TEST_F(MessageHandlerTest, transceiverRDMDiscoveryRequest) {
  // Any data, doesn't have to be valid RDM
  const uint8_t rdm_reply[] = {1, 3, 4, 4, 5};

  const uint8_t frame_reply[] = {0, 0, 0, 0, 1, 3, 4, 4, 5};

  EXPECT_CALL(m_transport_mock,
              Send(kToken, COMMAND_RDM_DUB_REQUEST, RC_TX_ERROR, _, _))
      .With(Args<3, 4>(PayloadIs(
            reinterpret_cast<const uint8_t*>(&kEmptyDUBResponse),
            arraysize(kEmptyDUBResponse))))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken + 1, COMMAND_RDM_DUB_REQUEST, RC_OK, _, _))
      .With(Args<3, 4>(PayloadIs(
            reinterpret_cast<const uint8_t*>(&frame_reply),
            arraysize(frame_reply))))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken + 2, COMMAND_RDM_DUB_REQUEST, RC_RDM_TIMEOUT, _, _))
      .With(Args<3, 4>(PayloadIs(
            reinterpret_cast<const uint8_t*>(&kEmptyDUBResponse),
            arraysize(kEmptyDUBResponse))))
      .WillOnce(Return(true));

  SendEvent(kToken, T_OP_RDM_DUB, T_RESULT_TX_ERROR, NULL, 0);
  SendEvent(kToken + 1, T_OP_RDM_DUB, T_RESULT_RX_DATA,
            static_cast<const uint8_t*>(rdm_reply),
            arraysize(rdm_reply));
  SendEvent(kToken + 2, T_OP_RDM_DUB, T_RESULT_RX_TIMEOUT, NULL, 0);
}

TEST_F(MessageHandlerTest, transceiverRDMBroadcastRequest) {
  // Any data, doesn't have to be valid RDM
  const uint8_t rdm_reply[] = {1, 3, 4, 4, 5};
  const uint8_t frame_reply[] = {1, 3, 4, 4, 5};

  EXPECT_CALL(m_transport_mock,
              Send(kToken, COMMAND_RDM_BROADCAST_REQUEST, RC_TX_ERROR, _, _))
      .With(Args<3, 4>(EmptyPayload()))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken + 1, COMMAND_RDM_BROADCAST_REQUEST,
                   RC_RDM_BCAST_RESPONSE, _, _))
      .With(Args<3, 4>(PayloadIs(
              reinterpret_cast<const uint8_t*>(&frame_reply),
              arraysize(frame_reply))))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken + 2, COMMAND_RDM_BROADCAST_REQUEST, RC_OK,
                   _, _))
      .With(Args<3, 4>(EmptyPayload()))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken + 3, COMMAND_RDM_BROADCAST_REQUEST,
                   RC_RDM_INVALID_RESPONSE, _, _))
      .With(Args<3, 4>(EmptyPayload()))
      .WillOnce(Return(true));

  SendEvent(kToken, T_OP_RDM_BROADCAST, T_RESULT_TX_ERROR, NULL, 0);
  SendEvent(kToken + 1, T_OP_RDM_BROADCAST, T_RESULT_RX_DATA,
            static_cast<const uint8_t*>(rdm_reply),
            arraysize(rdm_reply));
  SendEvent(kToken + 2, T_OP_RDM_BROADCAST, T_RESULT_RX_TIMEOUT, NULL, 0);
  SendEvent(kToken + 3, T_OP_RDM_BROADCAST, T_RESULT_RX_INVALID, NULL, 0);
}

TEST_F(MessageHandlerTest, transceiverRDMRequestWithResponse) {
  // Any data, doesn't have to be valid RDM
  const uint8_t rdm_reply[] = {1, 3, 4, 4, 5};
  const uint8_t frame_reply[] = {0, 0, 0, 0, 0, 0, 1, 3, 4, 4, 5};

  EXPECT_CALL(m_transport_mock,
              Send(kToken, COMMAND_RDM_REQUEST, RC_TX_ERROR, _, _))
      .With(Args<3, 4>(PayloadIs(
              reinterpret_cast<const uint8_t*>(&kEmptyRDMResponse),
              arraysize(kEmptyRDMResponse))))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken + 1, COMMAND_RDM_REQUEST, RC_OK, _, _))
      .With(Args<3, 4>(PayloadIs(
              reinterpret_cast<const uint8_t*>(&frame_reply),
              arraysize(frame_reply))))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken + 2, COMMAND_RDM_REQUEST, RC_RDM_TIMEOUT, _, _))
      .With(Args<3, 4>(PayloadIs(
              reinterpret_cast<const uint8_t*>(&kEmptyRDMResponse),
              arraysize(kEmptyRDMResponse))))
      .WillOnce(Return(true));
  EXPECT_CALL(m_transport_mock,
              Send(kToken + 3, COMMAND_RDM_REQUEST, RC_RDM_INVALID_RESPONSE, _,
                   _))
      .With(Args<3, 4>(PayloadIs(
              reinterpret_cast<const uint8_t*>(&kEmptyRDMResponse),
              arraysize(kEmptyRDMResponse))))
      .WillOnce(Return(true));

  SendEvent(kToken, T_OP_RDM_WITH_RESPONSE, T_RESULT_TX_ERROR, NULL, 0);
  SendEvent(kToken + 1, T_OP_RDM_WITH_RESPONSE, T_RESULT_RX_DATA,
            static_cast<const uint8_t*>(rdm_reply),
            arraysize(rdm_reply));
  SendEvent(kToken + 2, T_OP_RDM_WITH_RESPONSE, T_RESULT_RX_TIMEOUT, NULL, 0);
  SendEvent(kToken + 3, T_OP_RDM_WITH_RESPONSE, T_RESULT_RX_INVALID, NULL, 0);
}
