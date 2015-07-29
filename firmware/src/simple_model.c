/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * simple_model.c
 * Copyright (C) 2015 Simon Newton
 */
#include "simple_model.h"

#include <stdlib.h>

#include "coarse_timer.h"
#include "constants.h"
#include "rdm_frame.h"
#include "rdm_responder.h"
#include "rdm_util.h"
#include "utils.h"

// Various constants
enum {SOFTWARE_VERSION = 0x00000000 };

static const uint16_t FLASH_FAST = 1000u;
static const uint16_t FLASH_SLOW = 10000u;
static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule LED Driver";
static const char SOFTWARE_LABEL[] = "Alpha";
static const char DEFAULT_DEVICE_LABEL[] = "Ja Rule";

static const ResponderDefinition RESPONDER_DEFINITION;

/*
 * @brief The simple model state.
 */
typedef struct {
  // Mute params
  CoarseTimer_Value mute_timer;
  PORTS_CHANNEL mute_port;
  PORTS_BIT_POS mute_bit;

  // Identify params
  CoarseTimer_Value identify_timer;
  PORTS_CHANNEL identify_port;
  PORTS_BIT_POS identify_bit;
} SimpleModel;

static SimpleModel g_simple_model;

// PID Handlers
// ----------------------------------------------------------------------------
static int SetIdentifyDevice(const RDMHeader *header,
                             const uint8_t *param_data) {
  bool previous_identify = g_responder.identify_on;
  int r = RDMResponder_SetIdentifyDevice(header, param_data);
  if (g_responder.identify_on == previous_identify) {
    return r;
  }
  if (g_responder.identify_on) {
    g_simple_model.identify_timer = CoarseTimer_GetTime();
    PLIB_PORTS_PinSet(PORTS_ID_0, g_simple_model.identify_port,
                      g_simple_model.identify_bit);
  } else {
    PLIB_PORTS_PinClear(PORTS_ID_0, g_simple_model.identify_port,
                        g_simple_model.identify_bit);
  }
  return r;
}

// Public Functions
// ----------------------------------------------------------------------------
void SimpleModel_Initialize(const SimpleModelSettings *settings) {
  g_simple_model.mute_port = settings->mute_port;
  g_simple_model.mute_bit = settings->mute_bit;

  g_simple_model.identify_timer = 0u;
  g_simple_model.identify_port = settings->identify_port;
  g_simple_model.identify_bit = settings->identify_bit;
}

static void SimpleModel_Activate() {
  g_simple_model.mute_timer = CoarseTimer_GetTime();

  // Initialize hardware
  PLIB_PORTS_PinDirectionOutputSet(
    PORTS_ID_0, g_simple_model.identify_port, g_simple_model.identify_bit);
  PLIB_PORTS_PinClear(PORTS_ID_0, g_simple_model.identify_port,
                      g_simple_model.identify_bit);

  PLIB_PORTS_PinDirectionOutputSet(
    PORTS_ID_0, g_simple_model.mute_port, g_simple_model.mute_bit);
  PLIB_PORTS_PinSet(PORTS_ID_0, g_simple_model.mute_port,
                    g_simple_model.mute_bit);
  g_responder.def = &RESPONDER_DEFINITION;
  RDMResponder_ResetToFactoryDefaults();
}

static void SimpleModel_Deactivate() {
  PLIB_PORTS_PinClear(PORTS_ID_0, g_simple_model.identify_port,
                      g_simple_model.identify_bit);
  PLIB_PORTS_PinClear(PORTS_ID_0, g_simple_model.mute_port,
                      g_simple_model.mute_bit);
}

static int SimpleModel_Ioctl(ModelIoctl command, uint8_t *data,
                             unsigned int length) {
  switch (command) {
    case IOCTL_GET_UID:
      if (length != UID_LENGTH) {
        return 0;
      }
      RDMResponder_GetUID(data);
      return 1;
    default:
      return 0;
  }
}

static int SimpleModel_HandleRequest(const RDMHeader *header,
                                     const uint8_t *param_data) {
  if (!RDMUtil_RequiresAction(g_responder.uid, header->dest_uid)) {
    return RDM_RESPONDER_NO_RESPONSE;
  }

  if (header->command_class == DISCOVERY_COMMAND) {
    bool previous_mute = g_responder.is_muted;
    int r = RDMResponder_HandleDiscovery(header, param_data);
    if (previous_mute != g_responder.is_muted) {
      if (g_responder.is_muted) {
        PLIB_PORTS_PinClear(PORTS_ID_0, g_simple_model.mute_port,
                            g_simple_model.mute_bit);
      } else {
        PLIB_PORTS_PinSet(PORTS_ID_0, g_simple_model.mute_port,
                          g_simple_model.mute_bit);
        g_simple_model.mute_timer = CoarseTimer_GetTime();
      }
    }
    return r;
  }

  uint16_t sub_device = ntohs(header->sub_device);

  // No subdevice support for now.
  if (sub_device != SUBDEVICE_ROOT && sub_device != SUBDEVICE_ALL) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  // This model has no sub devices.
  if (header->command_class == GET_COMMAND && sub_device == SUBDEVICE_ALL) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  return RDMResponder_DispatchPID(header, param_data);
}

static void SimpleModel_Tasks() {
  if (g_responder.identify_on) {
    if (CoarseTimer_HasElapsed(g_simple_model.identify_timer, FLASH_FAST)) {
      g_simple_model.identify_timer = CoarseTimer_GetTime();
      PLIB_PORTS_PinToggle(PORTS_ID_0, g_simple_model.identify_port,
                           g_simple_model.identify_bit);
    }
  }

  if (!g_responder.is_muted) {
    if (CoarseTimer_HasElapsed(g_simple_model.mute_timer, FLASH_SLOW)) {
      g_simple_model.mute_timer = CoarseTimer_GetTime();
      PLIB_PORTS_PinToggle(
        PORTS_ID_0, g_simple_model.mute_port,
        g_simple_model.mute_bit);
    }
  }
}

const ModelEntry SIMPLE_MODEL_ENTRY = {
  .model_id = BASIC_RESPONDER_MODEL_ID,
  .activate_fn = SimpleModel_Activate,
  .deactivate_fn = SimpleModel_Deactivate,
  .ioctl_fn = SimpleModel_Ioctl,
  .request_fn = SimpleModel_HandleRequest,
  .tasks_fn = SimpleModel_Tasks
};

static const PIDDescriptor PID_DESCRIPTORS[] = {
  {PID_SUPPORTED_PARAMETERS, RDMResponder_GetSupportedParameters, 0u,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_INFO, RDMResponder_GetDeviceInfo, 0u, (PIDCommandHandler) NULL},
  {PID_PRODUCT_DETAIL_ID_LIST, RDMResponder_GetProductDetailIds, 0u,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_MODEL_DESCRIPTION, RDMResponder_GetDeviceModelDescription, 0u,
    (PIDCommandHandler) NULL},
  {PID_MANUFACTURER_LABEL, RDMResponder_GetManufacturerLabel, 0u,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_LABEL, RDMResponder_GetDeviceLabel, 0u,
    RDMResponder_SetDeviceLabel},
  {PID_SOFTWARE_VERSION_LABEL, RDMResponder_GetSoftwareVersionLabel, 0u,
    (PIDCommandHandler) NULL},
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0u, SetIdentifyDevice}
};

static const ProductDetailIds PRODUCT_DETAIL_ID_LIST = {
  .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL},
  .size = 2u
};

static const ResponderDefinition RESPONDER_DEFINITION = {
  .descriptors = PID_DESCRIPTORS,
  .descriptor_count = sizeof(PID_DESCRIPTORS) / sizeof(PIDDescriptor),
  .sensors = NULL,
  .sensor_count = 0,
  .personalities = NULL,
  .personality_count = 0u,
  .software_version_label = SOFTWARE_LABEL,
  .manufacturer_label = MANUFACTURER_LABEL,
  .model_description = DEVICE_MODEL_DESCRIPTION,
  .product_detail_ids = &PRODUCT_DETAIL_ID_LIST,
  .default_device_label = DEFAULT_DEVICE_LABEL,
  .software_version = SOFTWARE_VERSION,
  .model_id = BASIC_RESPONDER_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT,
};
