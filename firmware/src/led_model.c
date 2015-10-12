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
 * led_model.c
 * Copyright (C) 2015 Simon Newton
 */
#include "led_model.h"

#include <stdlib.h>

#include "constants.h"
#include "macros.h"
#include "rdm_frame.h"
#include "rdm_responder.h"
#include "rdm_util.h"
#include "utils.h"

// Various constants
enum {SOFTWARE_VERSION = 0x00000000 };

static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule LED Driver";
static const char SOFTWARE_LABEL[] = "Alpha";
static const char DEFAULT_DEVICE_LABEL[] = "Ja Rule";
enum { MAX_PIXEL_COUNT = 170u };
enum { DEFAULT_PIXEL_COUNT = 2u };

static const ResponderDefinition RESPONDER_DEFINITION;

typedef enum {
  PIXEL_TYPE_LPD8806 = 0x0001,
  /*
  PIXEL_TYPE_WS2801 = 0x0002,
  PIXEL_TYPE_P9813 = 0x0003,
  PIXEL_TYPE_APA102 = 0x0004,
  */
} PixelType;

typedef struct {
  PixelType pixel_type;

  /**
   * @brief The number of pixels.
   *
   * This is a uint16_t in case we ever want to support non-RGB pixels. In that
   * case we could have up to 512 of them.
   */
  uint16_t pixel_count;
} LEDModel;


static const char PIXEL_TYPE_STRING[] = "Pixel Type";
static const char PIXEL_COUNT_STRING[] = "Pixel Count";

static const ParameterDescription PIXEL_TYPE_DESCRIPTION = {
  .pdl_size = 2u,
  .data_type = DS_UNSIGNED_WORD,
  .command_class = CC_GET_SET,
  .unit = UNITS_NONE,
  .prefix = PREFIX_NONE,
  .min_valid_value = PIXEL_TYPE_LPD8806,
  .max_valid_value = PIXEL_TYPE_LPD8806,
  .default_value = PIXEL_TYPE_LPD8806,
  .description = PIXEL_TYPE_STRING,
};

static const ParameterDescription PIXEL_COUNT_DESCRIPTION = {
  .pdl_size = 2u,
  .data_type = DS_UNSIGNED_WORD,
  .command_class = CC_GET_SET,
  .unit = UNITS_NONE,
  .prefix = PREFIX_NONE,
  .min_valid_value = 1u,
  .max_valid_value = MAX_PIXEL_COUNT,
  .default_value = DEFAULT_PIXEL_COUNT,
  .description = PIXEL_COUNT_STRING,
};

static LEDModel g_model;

// PID Handlers
// ----------------------------------------------------------------------------
int LEDModel_GetParameterDescription(const RDMHeader *header,
                                     UNUSED const uint8_t *param_data) {
  const uint16_t param_id = ExtractUInt16(param_data);
  const ParameterDescription *description = NULL;
  switch (param_id) {
    case PID_PIXEL_TYPE:
      description = &PIXEL_TYPE_DESCRIPTION;
      break;
    case PID_PIXEL_COUNT:
      description = &PIXEL_COUNT_DESCRIPTION;
      break;
    default:
      {}
  }
  if (description) {
    return RDMResponder_BuildParamDescription(header, param_id, description);
  } else {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }
}

int LEDModel_GetPixelType(const RDMHeader *header,
                          UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetUInt16(header, g_model.pixel_type);
}

int LEDModel_SetPixelType(const RDMHeader *header,
                          const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint16_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  const uint16_t type = ExtractUInt16(param_data);
  if (type != PIXEL_TYPE_LPD8806) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }
  g_model.pixel_type = type;
  return RDMResponder_BuildSetAck(header);
}

int LEDModel_GetPixelCount(const RDMHeader *header,
                           UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetUInt16(header, g_model.pixel_count);
}

int LEDModel_SetPixelCount(const RDMHeader *header,
                              const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint16_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t count = ExtractUInt16(param_data);
  if (count > MAX_PIXEL_COUNT) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }
  g_model.pixel_count = count;
  return RDMResponder_BuildSetAck(header);
}

// Public Functions
// ----------------------------------------------------------------------------
void LEDModel_Initialize() {}

static void LEDModel_Activate() {
  g_responder->def = &RESPONDER_DEFINITION;
  RDMResponder_InitResponder();
  g_model.pixel_type = PIXEL_TYPE_LPD8806;
  g_model.pixel_count = DEFAULT_PIXEL_COUNT;
}

static void LEDModel_Deactivate() {}

static int LEDModel_HandleRequest(const RDMHeader *header,
                                  const uint8_t *param_data) {
  if (!RDMUtil_RequiresAction(g_responder->uid, header->dest_uid)) {
    return RDM_RESPONDER_NO_RESPONSE;
  }

  if (header->command_class == DISCOVERY_COMMAND) {
    return RDMResponder_HandleDiscovery(header, param_data);
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

static void LEDModel_Tasks() {}

const ModelEntry LED_MODEL_ENTRY = {
  .model_id = LED_MODEL_ID,
  .activate_fn = LEDModel_Activate,
  .deactivate_fn = LEDModel_Deactivate,
  .ioctl_fn = RDMResponder_Ioctl,
  .request_fn = LEDModel_HandleRequest,
  .tasks_fn = LEDModel_Tasks
};

static const PIDDescriptor PID_DESCRIPTORS[] = {
  {PID_SUPPORTED_PARAMETERS, RDMResponder_GetSupportedParameters, 0u,
    (PIDCommandHandler) NULL},
  {PID_PARAMETER_DESCRIPTION, LEDModel_GetParameterDescription, 2u,
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
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0u,
    RDMResponder_SetIdentifyDevice},
  {PID_PIXEL_TYPE, LEDModel_GetPixelType, 0u, LEDModel_SetPixelType},
  {PID_PIXEL_COUNT, LEDModel_GetPixelCount, 0u, LEDModel_SetPixelCount}
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
  .model_id = LED_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT,
};
