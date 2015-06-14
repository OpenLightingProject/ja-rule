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
 * RDMResponderMock.h
 * A mock RDMResponder module.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_RDMRESPONDERMOCK_H_
#define TESTS_MOCKS_RDMRESPONDERMOCK_H_

#include <gmock/gmock.h>

#include "rdm_responder.h"

class MockRDMResponder {
 public:
  MOCK_METHOD2(Initialize, void(const uint8_t uid[UID_LENGTH],
                                RDMResponderSendCallback send_callback));
  MOCK_METHOD1(UIDRequiresAction, bool(const uint8_t uid[UID_LENGTH]));
  MOCK_METHOD2(VerifyChecksum, bool(const uint8_t *frame, unsigned int size));
  MOCK_METHOD3(HandleRequest, void(const RDMHeader *header,
                                   const uint8_t *param_data,
                                   unsigned int length));
  MOCK_METHOD0(IsMuted, bool());
};

void RDMResponder_SetMock(MockRDMResponder* mock);

#endif  // TESTS_MOCKS_RDMRESPONDERMOCK_H_
