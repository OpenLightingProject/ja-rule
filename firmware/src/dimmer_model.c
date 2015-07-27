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
 * dimmer_model.c
 * Copyright (C) 2015 Simon Newton
 */
#include "dimmer_model.h"

#include <stdlib.h>

#include "constants.h"
#include "macros.h"
#include "rdm_frame.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"
#include "rdm_util.h"
#include "utils.h"

#include <system_config.h>

// Various constants
enum { NUMBER_OF_SUB_DEVICES = 4 };
enum { PERSONALITY_COUNT = 1 };
enum { SOFTWARE_VERSION = 0x00000000 };
static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule Dimmer Device";
static const char SOFTWARE_LABEL[] = "Alpha";
static const char DEFAULT_DEVICE_LABEL[] = "Ja Rule";
static const char PERSONALITY_DESCRIPTION[] = "Dimmer";
static const uint16_t INITIAL_START_ADDRESSS = 1;


static const ResponderDefinition ROOT_RESPONDER_DEFINITION;
static const ResponderDefinition SUBDEVICE_RESPONDER_DEFINITION;

typedef struct {
  RDMResponder responder;
  uint16_t index;

  uint8_t identify_mode;
} DimmerSubDevice;

static DimmerSubDevice g_subdevices[NUMBER_OF_SUB_DEVICES];

DimmerSubDevice *g_active_device = NULL;

// Helper functions
// ----------------------------------------------------------------------------

/*
 * @brief Set a block address for all the sub devices.
 * @param start_address the new start address
 * @returns true if the start address of all subdevices changed, false if the
 *   footprint of the sub devices would exceed the last slot (512).
 */

bool ResetToBlockAddress(uint16_t start_address) {
  unsigned int footprint = 0;
  unsigned int i = 0;
  for (; i < NUMBER_OF_SUB_DEVICES; i++) {
    RDMResponder *responder = &g_subdevices[i].responder;
    footprint += responder->def->personalities[responder->current_personality - 1]
        .slot_count;
  }

  if (MAX_DMX_START_ADDRESS - start_address + 1 < footprint) {
    return false;
  }

  for (i = 0; i < NUMBER_OF_SUB_DEVICES; i++) {
    RDMResponder *responder = &g_subdevices[i].responder;
    responder->dmx_start_address = start_address;
    const PersonalityDefinition *personality =
        &responder->def->personalities[responder->current_personality - 1];
    start_address += personality->slot_count;
  }
  return true;
}

// PID Handlers
// ----------------------------------------------------------------------------
int DimmerModel_GetIdentifyMode(const RDMHeader *header,
                                UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetUInt8(header, g_active_device->identify_mode);
}

int DimmerModel_SetIdentifyMode(const RDMHeader *header,
                                const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint8_t mode = param_data[0];
  if (mode != IDENTIFY_MODE_QUIET && mode != IDENTIFY_MODE_LOUD) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_active_device->identify_mode = mode;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetDMXBlockAddress(const RDMHeader *header,
                                   UNUSED const uint8_t *param_data) {
  uint16_t total_footprint = 0;
  uint16_t expected_start_address = 0;
  bool is_contiguous = true;
  unsigned int i = 0;
  for (; i < NUMBER_OF_SUB_DEVICES; i++) {
    RDMResponder *responder = &g_subdevices[i].responder;
    uint16_t sub_device_footprint = responder->def
        ->personalities[responder->current_personality - 1].slot_count;
    total_footprint += sub_device_footprint;
    if (expected_start_address &&
        expected_start_address != responder->dmx_start_address) {
        is_contiguous = false;
    } else if (expected_start_address) {
      expected_start_address += sub_device_footprint;
    } else {
      expected_start_address = responder->dmx_start_address +
                               sub_device_footprint;
    }
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt16(ptr, total_footprint);
  ptr = PushUInt16(
      ptr,
      is_contiguous ? g_subdevices[0].responder.dmx_start_address :
          INVALID_DMX_START_ADDRESS);
  return RDMResponder_AddHeaderAndChecksum(header, ACK,
                                           ptr - g_rdm_buffer);
}

int DimmerModel_SetDMXBlockAddress(const RDMHeader *header,
                                   const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint16_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint16_t start_address = ExtractUInt16(param_data);
  if (start_address == 0 || start_address > MAX_DMX_START_ADDRESS) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  bool ok = ResetToBlockAddress(start_address);
  if (ok) {
    return RDMResponder_BuildSetAck(header);
  } else {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }
}


// Public Functions
// ----------------------------------------------------------------------------
void DimmerModel_Initialize() {
  RDMResponder *temp = g_responder;

  unsigned int i = 0;
  uint16_t sub_device_index = 1;
  for (; i < NUMBER_OF_SUB_DEVICES; i++) {
    if (i == 1) {
      // Leave at gap at sub-device 2, since sub devices aren't required to be
      // contiguous.
      sub_device_index++;
    }

    g_subdevices[i].index = sub_device_index++;
    g_subdevices[i].responder.def = &SUBDEVICE_RESPONDER_DEFINITION;
    g_responder = &g_subdevices[i].responder;
    memcpy(g_responder->uid, temp->uid, UID_LENGTH);
    RDMResponder_ResetToFactoryDefaults();
    g_responder->is_subdevice = true;
    g_responder->sub_device_count = NUMBER_OF_SUB_DEVICES;
  }

  // restore
  g_responder = temp;
  if (!ResetToBlockAddress(INITIAL_START_ADDRESSS)) {
    // Set them all to 1
    for (i = 0; i < NUMBER_OF_SUB_DEVICES; i++) {
      RDMResponder *responder = &g_subdevices[i].responder;
      responder->dmx_start_address = INITIAL_START_ADDRESSS;
    }
  }
}

static void DimmerModel_Activate() {
  g_responder->def = &ROOT_RESPONDER_DEFINITION;
  RDMResponder_ResetToFactoryDefaults();
  g_responder->sub_device_count = NUMBER_OF_SUB_DEVICES;
}

static void DimmerModel_Deactivate() {}

static int DimmerModel_Ioctl(ModelIoctl command, uint8_t *data,
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

static int DimmerModel_HandleRequest(const RDMHeader *header,
                                     const uint8_t *param_data) {
  if (!RDMUtil_RequiresAction(g_responder->uid, header->dest_uid)) {
    return RDM_RESPONDER_NO_RESPONSE;
  }

  // The standard isn't at all clear how a responder is supposed to behave if
  // it receives discovery commands with a non-0 subdevice. For now we just
  // ignore the sub-device field.
  if (header->command_class == DISCOVERY_COMMAND) {
    return RDMResponder_HandleDiscovery(header, param_data);
  }

  const uint16_t sub_device = ntohs(header->sub_device);

  // GETs to all subdevices are invalid.
  if (header->command_class == GET_COMMAND && sub_device == SUBDEVICE_ALL) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  if (sub_device == SUBDEVICE_ROOT) {
    return RDMResponder_DispatchPID(header, param_data);
  }

  RDMResponder *temp = g_responder;
  unsigned int i = 0;
  bool handled = false;
  int response_size = RDM_RESPONDER_NO_RESPONSE;
  for (; i < NUMBER_OF_SUB_DEVICES; i++) {
    if (sub_device == g_subdevices[i].index || sub_device == SUBDEVICE_ALL) {
      g_active_device = &g_subdevices[i];
      g_responder = &g_subdevices[i].responder;
      response_size = RDMResponder_DispatchPID(header, param_data);
      handled = true;
    }
  }

  // restore
  g_responder = temp;

  if (!handled) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  // If it was an all-subdevices call, it's not really clear how to handle the
  // response, in this case we return the last one.
  return response_size;
}

static void DimmerModel_Tasks() {}

const ModelEntry DIMMER_MODEL_ENTRY = {
  .model_id = DIMMER_MODEL_ID,
  .activate_fn = DimmerModel_Activate,
  .deactivate_fn = DimmerModel_Deactivate,
  .ioctl_fn = DimmerModel_Ioctl,
  .request_fn = DimmerModel_HandleRequest,
  .tasks_fn = DimmerModel_Tasks
};

// Root device definition
// ----------------------------------------------------------------------------

static const PIDDescriptor ROOT_PID_DESCRIPTORS[] = {
  {PID_SUPPORTED_PARAMETERS, RDMResponder_GetSupportedParameters, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_INFO, RDMResponder_GetDeviceInfo, 0, (PIDCommandHandler) NULL},
  {PID_PRODUCT_DETAIL_ID_LIST, RDMResponder_GetProductDetailIds, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_MODEL_DESCRIPTION, RDMResponder_GetDeviceModelDescription, 0,
    (PIDCommandHandler) NULL},
  {PID_MANUFACTURER_LABEL, RDMResponder_GetManufacturerLabel, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_LABEL, RDMResponder_GetDeviceLabel, 0,
    RDMResponder_SetDeviceLabel},
  {PID_SOFTWARE_VERSION_LABEL, RDMResponder_GetSoftwareVersionLabel, 0,
    (PIDCommandHandler) NULL},
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0,
    RDMResponder_SetIdentifyDevice},
  {PID_DMX_BLOCK_ADDRESS, DimmerModel_GetDMXBlockAddress, 0,
    DimmerModel_SetDMXBlockAddress}
};

static const ProductDetailIds ROOT_PRODUCT_DETAIL_ID_LIST = {
  .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL },
  .size = 2
};

static const ResponderDefinition ROOT_RESPONDER_DEFINITION = {
  .descriptors = ROOT_PID_DESCRIPTORS,
  .descriptor_count = sizeof(ROOT_PID_DESCRIPTORS) / sizeof(PIDDescriptor),
  .sensors = NULL,
  .sensor_count = 0,
  .personalities = NULL,
  .personality_count = 0,
  .software_version_label = SOFTWARE_LABEL,
  .manufacturer_label = MANUFACTURER_LABEL,
  .model_description = DEVICE_MODEL_DESCRIPTION,
  .product_detail_ids = &ROOT_PRODUCT_DETAIL_ID_LIST,
  .default_device_label = DEFAULT_DEVICE_LABEL,
  .software_version = SOFTWARE_VERSION,
  .model_id = DIMMER_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT
};


// Sub device definitions
// ----------------------------------------------------------------------------

static const PIDDescriptor SUBDEVICE_PID_DESCRIPTORS[] = {
  {PID_SUPPORTED_PARAMETERS, RDMResponder_GetSupportedParameters, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_INFO, RDMResponder_GetDeviceInfo, 0, (PIDCommandHandler) NULL},
  {PID_PRODUCT_DETAIL_ID_LIST, RDMResponder_GetProductDetailIds, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_MODEL_DESCRIPTION, RDMResponder_GetDeviceModelDescription, 0,
    (PIDCommandHandler) NULL},
  {PID_MANUFACTURER_LABEL, RDMResponder_GetManufacturerLabel, 0,
    (PIDCommandHandler) NULL},
  {PID_DMX_START_ADDRESS, RDMResponder_GetDMXStartAddress, 0,
    RDMResponder_SetDMXStartAddress},
  {PID_SOFTWARE_VERSION_LABEL, RDMResponder_GetSoftwareVersionLabel, 0,
    (PIDCommandHandler) NULL},
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0,
    RDMResponder_SetIdentifyDevice},
  {PID_IDENTIFY_MODE, DimmerModel_GetIdentifyMode, 0,
    DimmerModel_SetIdentifyMode}
};

static const ProductDetailIds SUBDEVICE_PRODUCT_DETAIL_ID_LIST = {
  .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL},
  .size = 2
};

static const char SLOT_DIMMER_DESCRIPTION[] = "Dimmer";

static const SlotDefinition PERSONALITY_SLOTS[] = {
  {
    .description = SLOT_DIMMER_DESCRIPTION,
    .slot_label_id = SD_INTENSITY,
    .slot_type = ST_PRIMARY,
    .default_value = 0,
  }
};

static const PersonalityDefinition PERSONALITIES[PERSONALITY_COUNT] = {
  {
    .dmx_footprint = 1,
    .description = PERSONALITY_DESCRIPTION,
    .slots = PERSONALITY_SLOTS,
    .slot_count = 1
  },
};

static const ResponderDefinition SUBDEVICE_RESPONDER_DEFINITION = {
  .descriptors = SUBDEVICE_PID_DESCRIPTORS,
  .descriptor_count = sizeof(SUBDEVICE_PID_DESCRIPTORS) / sizeof(PIDDescriptor),
  .sensors = NULL,
  .sensor_count = 0,
  .personalities = PERSONALITIES,
  .personality_count = 1,
  .software_version_label = SOFTWARE_LABEL,
  .manufacturer_label = MANUFACTURER_LABEL,
  .model_description = DEVICE_MODEL_DESCRIPTION,
  .product_detail_ids = &SUBDEVICE_PRODUCT_DETAIL_ID_LIST,
  .default_device_label = DEFAULT_DEVICE_LABEL,
  .software_version = SOFTWARE_VERSION,
  .model_id = DIMMER_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT
};
