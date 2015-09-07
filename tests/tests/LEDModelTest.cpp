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
 * LEDModelTest.cpp
 * Tests for the LED Model RDM responder.
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

#include "led_model.h"
#include "rdm.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"
#include "Array.h"
#include "Matchers.h"
#include "ModelTest.h"
#include "TestHelpers.h"

using ola::network::HostToNetwork;
using ola::rdm::UID;
using ola::rdm::GetResponseFromData;
using ola::rdm::NackWithReason;
using ola::rdm::RDMRequest;
using ola::rdm::RDMResponse;
using ola::rdm::RDMSetRequest;
using std::unique_ptr;

class LEDModelTest : public ModelTest {
 public:
  LEDModelTest() : ModelTest(&LED_MODEL_ENTRY) {}

  void SetUp() {
    RDMResponderSettings settings;
    memcpy(settings.uid, TEST_UID, UID_LENGTH);
    RDMResponder_Initialize(&settings);
    LEDModel_Initialize();
    LED_MODEL_ENTRY.activate_fn();
  }
};
