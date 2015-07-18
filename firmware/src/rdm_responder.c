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
 * rdm_responder.c
 * Copyright (C) 2015 Simon Newton
 */
#include "rdm_responder.h"

#if HAVE_CONFIG_H
// We're in the test environment
#include <config.h>
#else
#include <machine/endian.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <string.h>

#include "constants.h"
#include "macros.h"
#include "rdm_buffer.h"
#include "rdm_util.h"
#include "utils.h"

const char MANUFACTURER_LABEL[] = "Open Lighting Project";

// Microchip defines this macro in stdlib.h but it's non standard.
// We define it here so that the unit tests work.
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define FIVE5_CONSTANT 0x55
#define AA_CONSTANT 0xaa
#define FE_CONSTANT 0xfe

RDMResponder g_responder;

#define ReturnUnlessUnicast(header) \
if (!RDMUtil_RequiresResponse(&g_responder, header->dest_uid)) {\
  return RDM_RESPONDER_NO_RESPONSE; \
}

void RDMResponder_Initialize(const uint8_t uid[UID_LENGTH]) {
  memcpy(g_responder.uid, uid, UID_LENGTH);
  g_responder.def = NULL;
  RDMResponder_ResetToFactoryDefaults();
}

void RDMResponder_ResetToFactoryDefaults() {
  g_responder.queued_message_count = 0;
  g_responder.dmx_start_address = INVALID_DMX_START_ADDRESS;
  g_responder.dmx_footprint = 0;
  g_responder.sub_device_count = 0;
  g_responder.sensor_count = 0;
  g_responder.current_personality = 0;
  g_responder.personality_count = 0;
  g_responder.is_muted = false;
  g_responder.identify_on = false;

  if (g_responder.def) {
    strncpy(g_responder.device_label,
            g_responder.def->default_device_label,
            RDMUtil_SafeStringLength(g_responder.def->default_device_label,
                                     RDM_DEFAULT_STRING_SIZE));
    g_responder.device_label[RDM_DEFAULT_STRING_SIZE] = 0;
  }

  g_responder.using_factory_defaults = true;
}

void RDMResponder_GetUID(uint8_t *uid) {
  memcpy(uid, g_responder.uid, UID_LENGTH);
}

int RDMResponder_HandleDUBRequest(const uint8_t *param_data,
                                  unsigned int param_data_length) {
  if (g_responder.is_muted || param_data_length != 2 * UID_LENGTH) {
    return RDM_RESPONDER_NO_RESPONSE;
  }

  if (!(RDMUtil_UIDCompare(param_data, g_responder.uid) <= 0 &&
        RDMUtil_UIDCompare(g_responder.uid, param_data + UID_LENGTH) <= 0)) {
    return RDM_RESPONDER_NO_RESPONSE;
  }

  uint8_t *response = (uint8_t*) g_rdm_buffer;
  memset(response, FE_CONSTANT, 7);
  response[7] = AA_CONSTANT;
  response[8] = g_responder.uid[0] | AA_CONSTANT;
  response[9] = g_responder.uid[0] | FIVE5_CONSTANT;
  response[10] = g_responder.uid[1] | AA_CONSTANT;
  response[11] = g_responder.uid[1] | FIVE5_CONSTANT;

  response[12] = g_responder.uid[2] | AA_CONSTANT;
  response[13] = g_responder.uid[2] | FIVE5_CONSTANT;
  response[14] = g_responder.uid[3] | AA_CONSTANT;
  response[15] = g_responder.uid[3] | FIVE5_CONSTANT;
  response[16] = g_responder.uid[4] | AA_CONSTANT;
  response[17] = g_responder.uid[4] | FIVE5_CONSTANT;
  response[18] = g_responder.uid[5] | AA_CONSTANT;
  response[19] = g_responder.uid[5] | FIVE5_CONSTANT;

  uint16_t checksum = 0;
  unsigned int i;
  for (i = 8; i < 20; i++) {
    checksum += response[i];
  }

  response[20] = ShortMSB(checksum) | AA_CONSTANT;
  response[21] = ShortMSB(checksum) | FIVE5_CONSTANT;
  response[22] = ShortLSB(checksum) | AA_CONSTANT;
  response[23] = ShortLSB(checksum) | FIVE5_CONSTANT;
  return -DUB_RESPONSE_LENGTH;
}

void RDMResponder_BuildHeader(const RDMHeader *incoming_header,
                              RDMResponseType response_type,
                              RDMCommandClass command_class,
                              RDMPid pid,
                              unsigned int param_data_length) {
  RDMHeader *outgoing_header = (RDMHeader*) g_rdm_buffer;
  outgoing_header->start_code = RDM_START_CODE;
  outgoing_header->sub_start_code = SUB_START_CODE;
  outgoing_header->message_length = sizeof(RDMHeader) + param_data_length;
  memcpy(outgoing_header->dest_uid, incoming_header->src_uid, UID_LENGTH);
  memcpy(outgoing_header->src_uid, g_responder.uid, UID_LENGTH);
  outgoing_header->transaction_number = incoming_header->transaction_number;
  outgoing_header->port_id = response_type;
  outgoing_header->message_count = g_responder.queued_message_count;
  outgoing_header->sub_device = incoming_header->sub_device;
  outgoing_header->command_class = command_class;
  outgoing_header->param_id = htons(pid);
  outgoing_header->param_data_length = param_data_length;
}

int RDMResponder_BuildNack(const RDMHeader *header, RDMNackReason reason) {
  ReturnUnlessUnicast(header);

  uint16_t param_data = htons(reason);

  RDMResponder_BuildHeader(
      header, NACK_REASON,
      (header->command_class == GET_COMMAND ?
       GET_COMMAND_RESPONSE : SET_COMMAND_RESPONSE),
       ntohs(header->param_id),
       sizeof(param_data));
  memcpy(g_rdm_buffer + sizeof(RDMHeader), &param_data, sizeof(param_data));
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_DispatchPID(const RDMHeader *header,
                             const uint8_t *param_data) {
  const ResponderDefinition *definition = g_responder.def;
  uint16_t pid = ntohs(header->param_id);
  unsigned int i = 0;
  // TODO(simon): convert to binary search if the list gets long.
  // We'll need to add a check to ensure it's sorted though.
  for (; i < definition->descriptor_count; i++) {
    if (pid == definition->descriptors[i].pid) {
      if (header->command_class == GET_COMMAND) {
        if (definition->descriptors[i].get_handler) {
          return definition->descriptors[i].get_handler(header, param_data);
        } else {
          return RDMResponder_BuildNack(header, NR_UNSUPPORTED_COMMAND_CLASS);
        }
      } else {
        if (definition->descriptors[i].set_handler) {
          return definition->descriptors[i].set_handler(header, param_data);
        } else {
          return RDMResponder_BuildNack(header, NR_UNSUPPORTED_COMMAND_CLASS);
        }
      }
    }
  }
  return RDMResponder_BuildNack(header, NR_UNKNOWN_PID);
}

// PID Handlers
// ----------------------------------------------------------------------------
int RDMResponder_GenericReturnString(const RDMHeader *header,
                                     const char *reply_string,
                                     unsigned int max_size) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  ReturnUnlessUnicast(header);

  unsigned int length = RDMUtil_SafeStringLength(reply_string, max_size);
  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id), length);
  memcpy(g_rdm_buffer + sizeof(RDMHeader), reply_string, length);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_SetMute(const RDMHeader *header) {
  if (header->param_data_length) {
    return RDM_RESPONDER_NO_RESPONSE;
  }
  g_responder.is_muted = true;

  ReturnUnlessUnicast(header);
  RDMResponder_BuildHeader(header, ACK, DISCOVERY_COMMAND_RESPONSE,
                           PID_DISC_MUTE, sizeof(uint16_t));
  uint8_t *param_data = g_rdm_buffer + sizeof(RDMHeader);
  param_data[0] = 0;  // set control field to 0
  param_data[1] = 0;  // set control field to 0
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

// TODO(simon): combine with RDMResponder_SetMute above.
int RDMResponder_SetUnMute(const RDMHeader *header) {
  if (header->param_data_length) {
    return RDM_RESPONDER_NO_RESPONSE;
  }
  g_responder.is_muted = false;

  ReturnUnlessUnicast(header);
  RDMResponder_BuildHeader(header, ACK, DISCOVERY_COMMAND_RESPONSE,
                           PID_DISC_UN_MUTE, sizeof(uint16_t));
  uint8_t *param_data = g_rdm_buffer + sizeof(RDMHeader);
  param_data[0] = 0;  // set control field to 0
  param_data[1] = 0;  // set control field to 0
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GetSupportedParameters(const RDMHeader *header,
                                        UNUSED const uint8_t *param_data) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  ReturnUnlessUnicast(header);
  const ResponderDefinition *definition = g_responder.def;

  unsigned int i = 0;
  unsigned int offset = sizeof(RDMHeader);
  for (; i < definition->descriptor_count; i++) {
    switch (definition->descriptors[i].pid) {
      case PID_DISC_UNIQUE_BRANCH:
      case PID_DISC_MUTE:
      case PID_DISC_UN_MUTE:
      case PID_SUPPORTED_PARAMETERS:
      case PID_PARAMETER_DESCRIPTION:
      case PID_DEVICE_INFO:
      case PID_SOFTWARE_VERSION_LABEL:
      case PID_DMX_START_ADDRESS:
      case PID_IDENTIFY_DEVICE:
        break;
      default:
        g_rdm_buffer[offset++] = ShortMSB(definition->descriptors[i].pid);
        g_rdm_buffer[offset++] = ShortLSB(definition->descriptors[i].pid);
    }
  }

  // TODO(simon): handle ack-overflow here
  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           PID_SUPPORTED_PARAMETERS,
                           offset - sizeof(RDMHeader));
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GetDeviceInfo(const RDMHeader *header,
                               UNUSED const uint8_t *param_data) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  ReturnUnlessUnicast(header);

  struct device_info_s {
    uint16_t rdm_version;
    uint16_t model;
    uint16_t product_category;
    uint32_t software_version;
    uint16_t dmx_footprint;
    uint8_t current_personality;
    uint8_t personality_count;
    uint16_t dmx_start_address;
    uint16_t sub_device_count;
    uint8_t sensor_count;
  } __attribute__((packed));

  struct device_info_s device_info = {
    .rdm_version = htons(RDM_VERSION),
    .model = htons(g_responder.def->model_id),
    .product_category = htons(g_responder.def->product_category),
    .software_version = htonl(g_responder.def->software_version),
    .dmx_footprint = htons(g_responder.dmx_footprint),
    .current_personality = g_responder.current_personality,
    .personality_count = g_responder.personality_count,
    .dmx_start_address = htons(g_responder.dmx_start_address),
    .sub_device_count = htons(g_responder.sub_device_count),
    .sensor_count = g_responder.sensor_count
  };

  /*
  // TODO make this .foo format
  struct device_info_s device_info;
  device_info.rdm_version = htons(RDM_VERSION);
  device_info.model = htons(g_responder.def->model_id);
  device_info.product_category = htons(g_responder.def->product_category);
  device_info.software_version = htonl(g_responder.def->software_version);
  device_info.dmx_footprint = htons(g_responder.dmx_footprint);
  device_info.current_personality = g_responder.current_personality;
  device_info.personality_count = g_responder.personality_count;
  device_info.dmx_start_address = htons(g_responder.dmx_start_address);
  device_info.sub_device_count = htons(g_responder.sub_device_count);
  device_info.sensor_count = g_responder.sensor_count;
  */

  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           PID_DEVICE_INFO, sizeof(device_info));
  memcpy(g_rdm_buffer + sizeof(RDMHeader),
         (const uint8_t*) &device_info, sizeof(device_info));
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GetProductDetailIds(const RDMHeader *header,
                                     UNUSED const uint8_t *param_data) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  ReturnUnlessUnicast(header);

  const ResponderDefinition *definition = g_responder.def;
  unsigned int size = 0;
  unsigned int offset = sizeof(RDMHeader);
  if (definition->product_detail_ids) {
    unsigned int i = 0;
    while (i < min(definition->product_detail_ids->size, MAX_PRODUCT_DETAILS)) {
      uint16_t detail_id = htons(definition->product_detail_ids->ids[i]);
      memcpy(g_rdm_buffer + offset, &detail_id, sizeof(RDMProductDetail));
      offset += sizeof(RDMProductDetail);
      size += sizeof(RDMProductDetail);
      i++;
    }
  }

  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           PID_PRODUCT_DETAIL_ID_LIST, size);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GetDeviceModelDescription(const RDMHeader *header,
                                           UNUSED const uint8_t *param_data) {
  const ResponderDefinition *definition = g_responder.def;
  return RDMResponder_GenericReturnString(header, definition->model_description,
                                          RDM_DEFAULT_STRING_SIZE);
}

int RDMResponder_GetManufacturerLabel(const RDMHeader *header,
                                      UNUSED const uint8_t *param_data) {
  const ResponderDefinition *definition = g_responder.def;
  return RDMResponder_GenericReturnString(header,
                                          definition->manufacturer_label,
                                          RDM_DEFAULT_STRING_SIZE);
}

int RDMResponder_GetSoftwareVersionLabel(const RDMHeader *header,
                                         UNUSED const uint8_t *param_data) {
  const ResponderDefinition *definition = g_responder.def;
  return RDMResponder_GenericReturnString(header,
                                          definition->software_version_label,
                                          RDM_DEFAULT_STRING_SIZE);
}

int RDMResponder_GetDeviceLabel(const RDMHeader *header,
                                UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericReturnString(header, g_responder.device_label,
                                          RDM_DEFAULT_STRING_SIZE);
}

int RDMResponder_SetDeviceLabel(const RDMHeader *header,
                                const uint8_t *param_data) {
  if (header->param_data_length > RDM_DEFAULT_STRING_SIZE) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  unsigned int len = min(header->param_data_length, RDM_DEFAULT_STRING_SIZE);
  strncpy(g_responder.device_label, (const char*) param_data, len);

  ReturnUnlessUnicast(header);
  RDMResponder_BuildHeader(header, ACK, SET_COMMAND_RESPONSE,
                           PID_DEVICE_LABEL, 0);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GetIdentifyDevice(const RDMHeader *header,
                                   UNUSED const uint8_t *param_data) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  ReturnUnlessUnicast(header);
  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           PID_IDENTIFY_DEVICE, sizeof(uint8_t));
  g_rdm_buffer[sizeof(RDMHeader)] = g_responder.identify_on;
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_SetIdentifyDevice(const RDMHeader *header,
                                   const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  switch (param_data[0]) {
    case 0:
      g_responder.identify_on = false;
      break;
    case 1:
      g_responder.identify_on = true;
      break;
    default:
      return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  ReturnUnlessUnicast(header);
  RDMResponder_BuildHeader(header, ACK, SET_COMMAND_RESPONSE,
                           PID_IDENTIFY_DEVICE, 0);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_HandleDiscovery(const RDMHeader *header,
                                 const uint8_t *param_data) {
  switch (ntohs(header->param_id)) {
    case PID_DISC_UNIQUE_BRANCH:
      return RDMResponder_HandleDUBRequest(param_data,
                                           header->param_data_length);
    case PID_DISC_MUTE:
      return RDMResponder_SetMute(header);
    case PID_DISC_UN_MUTE:
      return RDMResponder_SetUnMute(header);
    default:
      {}
  }
  return RDM_RESPONDER_NO_RESPONSE;
}
