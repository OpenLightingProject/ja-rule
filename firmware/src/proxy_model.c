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
// Must be at least 2.
enum { PROXY_BUFFERS_PER_CHILD = 2 };
enum { SOFTWARE_VERSION = 0x00000000 };
static const uint16_t ACK_TIMER_DELAY = 1u;
static const char DEFAULT_CHILD_DEVICE_LABEL[] = "Ja Rule Child Device";
static const char CHILD_DEVICE_MODEL_DESCRIPTION[] =
    "Ja Rule Proxy Child Device";
static const char DEFAULT_DEVICE_LABEL[] = "Ja Rule";
static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule Proxy Device";
static const char SOFTWARE_LABEL[] = "Alpha";

/*
 * Building a proxy is a bit tricky, because of the following requirement:
 *
 *   If the Status Type Requested is STATUS_GET_LAST_MESSAGE, the responder
 *   shall return the last message (which may be either a Queued Message or a
 *   Status Message) sent in response to a GET: QUEUED_MESSAGE.
 *
 * This means you need to be able to store at least two messages
 * simultaneously, the last queued message and the next queued message.
 *
 * And you need to do this for each device behind the proxy, since asking for
 * queued message for device A, shouldn't change the last queued message for
 * device B.
 *
 * So as a result we allocate PROXY_BUFFERS_PER_CHILD * NUMBER_OF_CHILDREN
 * buffers, where PROXY_BUFFERS_PER_CHILD >= 2.
 */

/*
 * @brief A proxy buffer
 */
typedef struct {
  uint8_t buffer[RDM_MAX_FRAME_SIZE];  // The data
} ProxyBuffer;

typedef struct {
  RDMResponder responder;

  ProxyBuffer buffers[PROXY_BUFFERS_PER_CHILD];  // The buffers for this child
  ProxyBuffer *last;  // Pointer to the last message for the child.
  ProxyBuffer *next;  // Pointer to the next message for the child.
  ProxyBuffer *free_list[PROXY_BUFFERS_PER_CHILD];  // Free list
  unsigned int free_size_count;  // Number of items on the free list.
} ChildDevice;

static ChildDevice g_children[NUMBER_OF_CHILDREN];

static const ResponderDefinition ROOT_RESPONDER_DEFINITION;
static const ResponderDefinition CHILD_DEVICE_RESPONDER_DEFINITION;

// Helper functions
// ----------------------------------------------------------------------------
void ResetProxyBuffers() {
  unsigned int i = 0u;
  for (; i < NUMBER_OF_CHILDREN; i++) {
    ChildDevice *device = &g_children[i];
    unsigned int j = 0u;
    for (; j != PROXY_BUFFERS_PER_CHILD; j++) {
      device->free_list[j] = &device->buffers[j];
    }
    device->next = NULL;
    device->last = NULL;
    device->free_size_count = NUMBER_OF_CHILDREN;
  }
}

static int HandleRequest(const RDMHeader *header, const uint8_t *param_data) {
  if (header->command_class == DISCOVERY_COMMAND) {
    return RDMResponder_HandleDiscovery(header, param_data);
  }

  if (ntohs(header->sub_device) != SUBDEVICE_ROOT) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  return RDMResponder_DispatchPID(header, param_data);
}

/*
 * @brief Respond with the queued message if appropriate.
 * @pre The request is unicast.
 */
static int MaybeRespondWithQueuedMessage(const RDMHeader *header,
                                         const uint8_t *param_data,
                                         unsigned int child_index) {
  if (header->param_data_length != sizeof(uint8_t) ||
      param_data[0] == STATUS_NONE ||
      param_data[0] > STATUS_ERROR) {
    // Malformed, let the child deal with it.
    return RDM_RESPONDER_NO_RESPONSE;
  }

  ChildDevice *device = &g_children[child_index];
  if (param_data[0] != STATUS_GET_LAST_MESSAGE && device->next) {
    // move next to last
    if (device->last) {
      device->free_list[device->free_size_count] = device->last;
      device->free_size_count++;
    }
    device->last = device->next;
    device->responder.queued_message_count = 0u;
    device->next = NULL;
  } else if (param_data[0] == STATUS_GET_LAST_MESSAGE && device->last) {
    // no op, going to return last
  } else {
    return RDM_RESPONDER_NO_RESPONSE;
  }

  RDMHeader *queued_header = (RDMHeader*) device->last->buffer;
  RDMResponder_BuildHeader(header, queued_header->port_id,
      queued_header->command_class, ntohs(queued_header->param_id),
      queued_header->message_length);
  memcpy(g_rdm_buffer + sizeof(RDMHeader),
         device->last->buffer + sizeof(RDMHeader),
         queued_header->message_length - sizeof(RDMHeader));
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

/*
 * @brief Handle a request for a child device.
 * @param header the incoming_header
 * @param param_data The param_data
 * @param child_index The child device index.
 */
static int HandleChildRequest(const RDMHeader *header,
                              const uint8_t *param_data,
                              unsigned int child_index) {
  if (header->command_class == DISCOVERY_COMMAND) {
    // Always pass discovery commands through to the child.
    return HandleRequest(header, param_data);
  }

  ChildDevice *device = &g_children[child_index];

  // If GET QUEUED_MESSAGE and there is next or last message, see if we need to
  // return it.
  int response_size = RDM_RESPONDER_NO_RESPONSE;
  if (header->command_class == GET_COMMAND &&
      ntohs(header->param_id) == PID_QUEUED_MESSAGE &&
      device->free_size_count != PROXY_BUFFERS_PER_CHILD &&
      RDMUtil_IsUnicast(header->dest_uid)) {
    response_size = MaybeRespondWithQueuedMessage(header, param_data,
                                                  child_index);
    if (response_size) {
      return response_size;
    }
  }

  // If the request is unicast, and we're out of buffer space then NACK.
  if (RDMUtil_IsUnicast(header->dest_uid) && device->next != NULL) {
    return RDMResponder_BuildNack(header, NR_PROXY_BUFFER_FULL);
  }

  // Let the child handle the request.
  response_size = HandleRequest(header, param_data);
  const RDMHeader *response_header = (RDMHeader*) g_rdm_buffer;
  // Only queue the frame if the message length is correct
  if (response_size >= (int) sizeof(RDMHeader) + (int) RDM_CHECKSUM_LENGTH &&
      ((int) response_header->message_length + (int) RDM_CHECKSUM_LENGTH ==
       response_size)) {
    if (device->next == NULL) {
      // Queue the response
      device->next = device->free_list[device->free_size_count - 1];
      device->free_size_count--;

      memcpy(device->next->buffer, g_rdm_buffer, response_size);
      response_size = RDMResponder_BuildAckTimer(header, ACK_TIMER_DELAY);
      g_responder->queued_message_count = 1u;
    } else {
      // Something has gone wrong, and we don't have space to queue the
      // response. Nack with a hardware fault.
      return RDMResponder_BuildNack(header, NR_HARDWARE_FAULT);
    }
  }
  return response_size;
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
int ProxyModelChild_GetQueuedMessage(const RDMHeader *header,
                                     UNUSED const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint8_t status_type = param_data[0];
  if (status_type == STATUS_NONE || status_type > STATUS_ERROR) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           PID_STATUS_MESSAGES, sizeof(RDMHeader));
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

// Public Functions
// ----------------------------------------------------------------------------
void ProxyModel_Initialize() {
  uint8_t parent_uid[UID_LENGTH];
  RDMResponder_GetUID(parent_uid);
  RDMResponder *temp = g_responder;

  // Initialize the child devices.
  unsigned int i = 0u;
  for (; i < NUMBER_OF_CHILDREN; i++) {
    ChildDevice *device = &g_children[i];
    g_responder = &device->responder;
    memcpy(g_responder->uid, parent_uid, UID_LENGTH);
    g_responder->uid[UID_LENGTH - 1] += (i + 1u);
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
  ResetProxyBuffers();
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

  RDMResponder *temp = g_responder;
  unsigned int i = 0u;
  for (; i < NUMBER_OF_CHILDREN; i++) {
    if (RDMUtil_RequiresAction(g_children[i].responder.uid, header->dest_uid)) {
      g_responder = &g_children[i].responder;
      int response_size = HandleChildRequest(header, param_data, i);
      g_responder = temp;
      if (response_size) {
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
  {PID_QUEUED_MESSAGE, ProxyModelChild_GetQueuedMessage, 1u,
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
  {PID_SOFTWARE_VERSION_LABEL, RDMResponder_GetSoftwareVersionLabel, 0u,
    (PIDCommandHandler) NULL},
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0u,
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
