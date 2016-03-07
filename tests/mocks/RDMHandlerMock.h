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
 * RDMHandlerMock.h
 * A mock RDMHandler module.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_RDMHANDLERMOCK_H_
#define TESTS_MOCKS_RDMHANDLERMOCK_H_

#include <gmock/gmock.h>

#include "rdm_handler.h"

class MockRDMHandler {
 public:
  MOCK_METHOD1(Initialize, void(const RDMHandlerSettings *settings));
  MOCK_METHOD1(AddModel, bool(const ModelEntry *entry));
  MOCK_METHOD1(SetActiveModel, bool(uint16_t model_id));
  MOCK_METHOD1(GetUID, void(uint8_t *uid));
  MOCK_METHOD2(HandleRequest, void(const RDMHeader *header,
                                   const uint8_t *param_data));
  MOCK_METHOD0(Tasks, void());
};

void RDMHandler_SetMock(MockRDMHandler* mock);

#endif  // TESTS_MOCKS_RDMHANDLERMOCK_H_
