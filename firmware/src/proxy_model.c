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
 * proxy_model.c
 * Copyright (C) 2015 Simon Newton
 */
#include "proxy_model.h"

#include <stdlib.h>

#include "constants.h"
#include "macros.h"
#include "rdm_frame.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"
#include "rdm_util.h"
#include "utils.h"

// Various constants
enum { NUMBER_OF_CHILDREN = 2 };
enum { SOFTWARE_VERSION = 0x00000000 };
static const char DEFAULT_CHILD_DEVICE_LABEL[] = "Ja Rule Child Device";
static const char CHILD_DEVICE_MODEL_DESCRIPTION[] =
    "Ja Rule Proxy Child Device";
static const char DEFAULT_DEVICE_LABEL[] = "Ja Rule";
static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule Proxy Device";
static const char SOFTWARE_LABEL[] = "Alpha";

typedef struct {
  RDMResponder responder;
} ChildDevice;

static ChildDevice g_children[NUMBER_OF_CHILDREN];

static const ResponderDefinition ROOT_RESPONDER_DEFINITION;
static const ResponderDefinition CHILD_DEVICE_RESPONDER_DEFINITION;

// Helper functions
// ----------------------------------------------------------------------------
static int HandleRequest(const RDMHeader *header, const uint8_t *param_data) {
  if (header->command_class == DISCOVERY_COMMAND) {
    return RDMResponder_HandleDiscovery(header, param_data);
  }

  if (ntohs(header->sub_device) != SUBDEVICE_ROOT) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  return RDMResponder_DispatchPID(header, param_data);
}

// Proxy PID Handlers
// ----------------------------------------------------------------------------
int ProxyModel_GetProxiedDeviceCount(const RDMHeader *header,
                                     UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt16(ptr, NUMBER_OF_CHILDREN);
  *ptr++ = 0;  // list change
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int ProxyModel_GetProxiedDevices(const RDMHeader *header,
                                 UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  unsigned int i = 0u;
  for (; i < NUMBER_OF_CHILDREN; i++) {
    memcpy(ptr, g_children[i].responder.uid, UID_LENGTH);
    ptr += UID_LENGTH;
  }
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

// Child PID Handlers
// ----------------------------------------------------------------------------

// Public Functions
// ----------------------------------------------------------------------------
void ProxyModel_Initialize() {
  RDMResponder *temp = g_responder;

  // Initialize the proxy

  // Initialize the child devices.
  uint8_t parent_uid[UID_LENGTH];
  RDMResponder_GetUID(parent_uid);

  unsigned int i = 0u;
  for (; i < NUMBER_OF_CHILDREN; i++) {

    ChildDevice *device = &g_children[i];
    g_responder = &device->responder;
    memcpy(g_responder->uid, parent_uid, UID_LENGTH);
    g_responder->uid[UID_LENGTH - 1] = i + 1u;
    g_responder->def = &CHILD_DEVICE_RESPONDER_DEFINITION;
    RDMResponder_ResetToFactoryDefaults();
    g_responder->is_proxied_device = true;
  }

  // restore
  g_responder = temp;
}

static void ProxyModel_Activate() {
  g_responder->def = &ROOT_RESPONDER_DEFINITION;
  RDMResponder_ResetToFactoryDefaults();
  g_responder->is_managed_proxy = true;
}

static void ProxyModel_Deactivate() {}

static int ProxyModel_HandleRequest(const RDMHeader *header,
                                    const uint8_t *param_data) {
  int response_size = 0;
  // The proxy always gets first dibs on responding.
  if (RDMUtil_RequiresAction(g_responder->uid, header->dest_uid)) {
    response_size = HandleRequest(header, param_data);
    if (response_size) {
      return response_size;
    }
  }

  unsigned int i = 0u;
  for (; i < NUMBER_OF_CHILDREN; i++) {
    if (RDMUtil_RequiresAction(g_children[i].responder.uid, header->dest_uid)) {
      RDMResponder *temp = g_responder;
      g_responder = &g_children[i].responder;
      response_size = HandleRequest(header, param_data);
      g_responder = temp;
      if (response_size) {
        // TODO(simon): make this more interesting by using ACK-TIMER in the
        // responses.
        return response_size;
      }
    }
  }
  return RDM_RESPONDER_NO_RESPONSE;
}

static void ProxyModel_Tasks() {}

const ModelEntry PROXY_MODEL_ENTRY = {
  .model_id = PROXY_MODEL_ID,
  .activate_fn = ProxyModel_Activate,
  .deactivate_fn = ProxyModel_Deactivate,
  .ioctl_fn = RDMResponder_Ioctl,
  .request_fn = ProxyModel_HandleRequest,
  .tasks_fn = ProxyModel_Tasks
};

// Root device definition
// ----------------------------------------------------------------------------

static const PIDDescriptor ROOT_PID_DESCRIPTORS[] = {
  {PID_PROXIED_DEVICES, ProxyModel_GetProxiedDevices, 0u,
    (PIDCommandHandler) NULL},
  {PID_PROXIED_DEVICE_COUNT, ProxyModel_GetProxiedDeviceCount, 0u,
    (PIDCommandHandler) NULL},
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
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0u,
    RDMResponder_SetIdentifyDevice},
};

static const ProductDetailIds ROOT_PRODUCT_DETAIL_ID_LIST = {
  .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL },
  .size = 2u
};

static const ResponderDefinition ROOT_RESPONDER_DEFINITION = {
  .descriptors = ROOT_PID_DESCRIPTORS,
  .descriptor_count = sizeof(ROOT_PID_DESCRIPTORS) / sizeof(PIDDescriptor),
  .sensors = NULL,
  .sensor_count = 0,
  .personalities = NULL,
  .personality_count = 0u,
  .software_version_label = SOFTWARE_LABEL,
  .manufacturer_label = MANUFACTURER_LABEL,
  .model_description = DEVICE_MODEL_DESCRIPTION,
  .product_detail_ids = &ROOT_PRODUCT_DETAIL_ID_LIST,
  .default_device_label = DEFAULT_DEVICE_LABEL,
  .software_version = SOFTWARE_VERSION,
  .model_id = PROXY_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT
};

// Child device definitions
// ----------------------------------------------------------------------------

static const PIDDescriptor CHILD_DEVICE_PID_DESCRIPTORS[] = {
  {PID_SUPPORTED_PARAMETERS, RDMResponder_GetSupportedParameters, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_INFO, RDMResponder_GetDeviceInfo, 0, (PIDCommandHandler) NULL},
  {PID_PRODUCT_DETAIL_ID_LIST, RDMResponder_GetProductDetailIds, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_MODEL_DESCRIPTION, RDMResponder_GetDeviceModelDescription, 0,
    (PIDCommandHandler) NULL},
  {PID_MANUFACTURER_LABEL, RDMResponder_GetManufacturerLabel, 0,
    (PIDCommandHandler) NULL},
  {PID_SOFTWARE_VERSION_LABEL, RDMResponder_GetSoftwareVersionLabel, 0,
    (PIDCommandHandler) NULL},
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0,
    RDMResponder_SetIdentifyDevice},
};

static const ProductDetailIds CHILD_DEVICE_PRODUCT_DETAIL_ID_LIST = {
  .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL},
  .size = 2u
};

static const ResponderDefinition CHILD_DEVICE_RESPONDER_DEFINITION = {
  .descriptors = CHILD_DEVICE_PID_DESCRIPTORS,
  .descriptor_count = sizeof(CHILD_DEVICE_PID_DESCRIPTORS) /
      sizeof(PIDDescriptor),
  .sensors = NULL,
  .sensor_count = 0u,
  .personalities = NULL,
  .personality_count = 0u,
  .software_version_label = SOFTWARE_LABEL,
  .manufacturer_label = MANUFACTURER_LABEL,
  .model_description = CHILD_DEVICE_MODEL_DESCRIPTION,
  .product_detail_ids = &CHILD_DEVICE_PRODUCT_DETAIL_ID_LIST,
  .default_device_label = DEFAULT_CHILD_DEVICE_LABEL,
  .software_version = SOFTWARE_VERSION,
  .model_id = PROXY_CHILD_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT
};
