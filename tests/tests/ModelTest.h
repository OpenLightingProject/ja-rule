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
 * ModelTest.h
 * Common base test for the Models.
 * Copyright (C) 2015 Simon Newton
 */

#include <gtest/gtest.h>

#include <ola/rdm/UID.h>
#include <ola/rdm/RDMCommand.h>
#include <memory>

#include "rdm_model.h"

#ifndef TESTS_TESTS_MODELTEST_H_
#define TESTS_TESTS_MODELTEST_H_

class ModelTest : public testing::Test {
 public:
  explicit ModelTest(const ModelEntry *model);

 protected:
  ola::rdm::UID m_controller_uid;
  ola::rdm::UID m_our_uid;
  const ModelEntry *m_model;

  static const uint8_t TEST_UID[UID_LENGTH];

  std::unique_ptr<ola::rdm::RDMRequest> BuildGetRequest(
      uint16_t pid,
      const uint8_t *param_data = nullptr,
      unsigned int param_data_size = 0);

  std::unique_ptr<ola::rdm::RDMRequest> BuildSetRequest(
      uint16_t pid,
      const uint8_t *param_data = nullptr,
      unsigned int param_data_size = 0);

  std::unique_ptr<ola::rdm::RDMRequest> BuildSubDeviceGetRequest(
      uint16_t pid,
      uint16_t sub_device,
      const uint8_t *param_data = nullptr,
      unsigned int param_data_size = 0);

  std::unique_ptr<ola::rdm::RDMRequest> BuildSubDeviceSetRequest(
      uint16_t pid,
      uint16_t sub_device,
      const uint8_t *param_data = nullptr,
      unsigned int param_data_size = 0);

  int InvokeRDMHandler(const ola::rdm::RDMRequest *request);
};

#endif   // TESTS_TESTS_MODELTEST_H_
