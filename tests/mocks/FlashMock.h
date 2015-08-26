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
 * FlashMock.h
 * A mock flash module.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_MOCKS_FLASHMOCK_H_
#define TESTS_MOCKS_FLASHMOCK_H_

#include <gmock/gmock.h>
#include "flash.h"

class FlashInterface {
 public:
  virtual ~FlashInterface() {}
  virtual bool ErasePage(uint32_t address) = 0;
  virtual bool WriteWord(uint32_t address, uint32_t data) = 0;
  virtual uint32_t ReadWord(uint32_t address) = 0;
};

class MockFlash : public FlashInterface {
 public:
  MOCK_METHOD1(ErasePage, bool(uint32_t address));
  MOCK_METHOD2(WriteWord, bool(uint32_t address, uint32_t data));
  MOCK_METHOD1(ReadWord, uint32_t(uint32_t address));
};

void Flash_SetMock(FlashInterface* mock);

#endif  // TESTS_MOCKS_FLASHMOCK_H_
