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
#define SENSOR_VALUE_PARAM_DATA_LENGTH 9

RDMResponder g_responder;

/*
 * @brief Record the sensor at the specified index.
 */
static inline void RecordSensor(unsigned int i) {
  if (g_responder.def->sensors[i].recorded_value_support &
      SENSOR_SUPPORTS_RECORDING_MASK) {
    g_responder.sensors[i].recorded_value =
        g_responder.sensors[i].present_value;
  }
}

/*
 * @brief Reset the sensor at the specified index.
 */
static void ResetSensor(unsigned int i) {
  if (g_responder.def->sensors[i].recorded_value_support &
      SENSOR_SUPPORTS_LOWEST_HIGHEST_MASK) {
    g_responder.sensors[i].lowest_value = g_responder.sensors[i].present_value;
    g_responder.sensors[i].highest_value = g_responder.sensors[i].present_value;
  } else {
    g_responder.sensors[i].lowest_value = SENSOR_VALUE_UNSUPPORTED;
    g_responder.sensors[i].highest_value = SENSOR_VALUE_UNSUPPORTED;
  }

  if (g_responder.def->sensors[i].recorded_value_support &
      SENSOR_SUPPORTS_RECORDING_MASK) {
    g_responder.sensors[i].recorded_value =
      g_responder.sensors[i].present_value;
  } else {
    g_responder.sensors[i].recorded_value = SENSOR_VALUE_UNSUPPORTED;
  }
}

/*
 * @brief Build a SENSOR_VALUE response.
 */
static void BuildSensorValueResponse(uint8_t index, const SensorData *sensor) {
  unsigned int offset = sizeof(RDMHeader);
  g_rdm_buffer[offset++] = index;
  g_rdm_buffer[offset++] = ShortMSB(sensor->present_value);
  g_rdm_buffer[offset++] = ShortLSB(sensor->present_value);
  g_rdm_buffer[offset++] = ShortMSB(sensor->lowest_value);
  g_rdm_buffer[offset++] = ShortLSB(sensor->lowest_value);
  g_rdm_buffer[offset++] = ShortMSB(sensor->highest_value);
  g_rdm_buffer[offset++] = ShortLSB(sensor->highest_value);
  g_rdm_buffer[offset++] = ShortMSB(sensor->recorded_value);
  g_rdm_buffer[offset++] = ShortLSB(sensor->recorded_value);
}

// Public Functions
// ----------------------------------------------------------------------------
void RDMResponder_Initialize(const uint8_t uid[UID_LENGTH]) {
  memcpy(g_responder.uid, uid, UID_LENGTH);
  g_responder.def = NULL;
  RDMResponder_ResetToFactoryDefaults();
}

void RDMResponder_ResetToFactoryDefaults() {
  g_responder.queued_message_count = 0;
  g_responder.dmx_start_address = INVALID_DMX_START_ADDRESS;
  g_responder.sub_device_count = 0;
  g_responder.current_personality = 0;
  g_responder.is_muted = false;
  g_responder.identify_on = false;
  g_responder.sensors = NULL;

  if (g_responder.def) {
    RDMUtil_StringCopy(g_responder.device_label, RDM_DEFAULT_STRING_SIZE,
                       g_responder.def->default_device_label,
                       RDM_DEFAULT_STRING_SIZE);
    if (g_responder.def->personality_count) {
      g_responder.current_personality = 1;
      g_responder.dmx_start_address = 1;
    }
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
                              uint16_t pid,
                              unsigned int param_data_length) {
  RDMHeader *outgoing_header = (RDMHeader*) g_rdm_buffer;
  outgoing_header->start_code = RDM_START_CODE;
  outgoing_header->sub_start_code = SUB_START_CODE;
  outgoing_header->message_length = sizeof(RDMHeader) + param_data_length;
  memcpy(outgoing_header->dest_uid, incoming_header->src_uid, UID_LENGTH);
  memcpy(outgoing_header->src_uid, incoming_header->dest_uid, UID_LENGTH);
  outgoing_header->transaction_number = incoming_header->transaction_number;
  outgoing_header->port_id = response_type;
  outgoing_header->message_count = g_responder.queued_message_count;
  outgoing_header->sub_device = incoming_header->sub_device;
  outgoing_header->command_class = command_class;
  outgoing_header->param_id = htons(pid);
  outgoing_header->param_data_length = param_data_length;
}

int RDMResponder_BuildSetAck(const RDMHeader *header) {
  ReturnUnlessUnicast(header);
  RDMResponder_BuildHeader(header, ACK, SET_COMMAND_RESPONSE,
                           ntohs(header->param_id), 0);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
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

int RDMResponder_GenericGetBool(const RDMHeader *header, bool value) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  ReturnUnlessUnicast(header);

  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id),
                           sizeof(uint8_t));
  g_rdm_buffer[sizeof(RDMHeader)] = value;
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}


int RDMResponder_GenericSetBool(const RDMHeader *header,
                                const uint8_t *param_data,
                                bool *value) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  switch (param_data[0]) {
    case 0:
      *value = false;
      break;
    case 1:
      *value = true;
      break;
    default:
      return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }
  return RDMResponder_BuildSetAck(header);
}

int RDMResponder_GenericGetUInt8(const RDMHeader *header, uint8_t value) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  ReturnUnlessUnicast(header);

  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id),
                           sizeof(uint8_t));
  g_rdm_buffer[sizeof(RDMHeader)] = value;
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GenericSetUInt8(const RDMHeader *header,
                                 const uint8_t *param_data,
                                 uint8_t *value) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  *value = param_data[0];
  return RDMResponder_BuildSetAck(header);
}

int RDMResponder_GenericGetUInt32(const RDMHeader *header, uint32_t value) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  ReturnUnlessUnicast(header);

  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id),
                           sizeof(uint32_t));
  g_rdm_buffer[sizeof(RDMHeader)] = (value & 0xff000000) >> 24;
  g_rdm_buffer[sizeof(RDMHeader) + 1] = (value & 0xff0000) >> 16;
  g_rdm_buffer[sizeof(RDMHeader) + 2] = (value & 0xff00) >> 8;
  g_rdm_buffer[sizeof(RDMHeader) + 3] = (value & 0xff);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GenericSetUInt32(const RDMHeader *header,
                                  const uint8_t *param_data,
                                  uint32_t *value) {
  if (header->param_data_length != sizeof(uint32_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  *value = (param_data[0] << 24) + (param_data[1] << 16) +
           (param_data[2] << 8) + param_data[3];
  return RDMResponder_BuildSetAck(header);
}

int RDMResponder_SetMute(const RDMHeader *header) {
  if (header->param_data_length) {
    return RDM_RESPONDER_NO_RESPONSE;
  }
  g_responder.is_muted = true;

  ReturnUnlessUnicast(header);
  RDMResponder_BuildHeader(header, ACK, DISCOVERY_COMMAND_RESPONSE,
                           ntohs(header->param_id), sizeof(uint16_t));
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
                           ntohs(header->param_id), sizeof(uint16_t));
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
                           ntohs(header->param_id), offset - sizeof(RDMHeader));
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

  uint16_t dmx_footprint = 0;
  if (g_responder.def->personalities) {
    dmx_footprint = g_responder.def->personalities[
        g_responder.current_personality - 1].dmx_footprint;
  }

  struct device_info_s device_info = {
    .rdm_version = htons(RDM_VERSION),
    .model = htons(g_responder.def->model_id),
    .product_category = htons(g_responder.def->product_category),
    .software_version = htonl(g_responder.def->software_version),
    .dmx_footprint = htons(dmx_footprint),
    .current_personality = g_responder.current_personality,
    .personality_count = g_responder.def->personality_count,
    .dmx_start_address = htons(g_responder.dmx_start_address),
    .sub_device_count = htons(g_responder.sub_device_count),
    .sensor_count = g_responder.def->sensor_count,
  };

  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id), sizeof(device_info));
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
                           ntohs(header->param_id), size);
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
  RDMUtil_StringCopy(g_responder.device_label, RDM_DEFAULT_STRING_SIZE,
                     (const char*) param_data, header->param_data_length);
  return RDMResponder_BuildSetAck(header);
}

int RDMResponder_GetDMXPersonality(const RDMHeader *header,
                                   UNUSED const uint8_t *param_data) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  ReturnUnlessUnicast(header);

  unsigned int offset = sizeof(RDMHeader);
  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id), sizeof(uint16_t));
  g_rdm_buffer[offset++] = g_responder.current_personality;
  g_rdm_buffer[offset++] = g_responder.def->personality_count;
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_SetDMXPersonality(const RDMHeader *header,
                                   const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint8_t new_personality = param_data[0];
  if (new_personality == 0 ||
      new_personality > g_responder.def->personality_count) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_responder.current_personality = new_personality;
  return RDMResponder_BuildSetAck(header);
}

int RDMResponder_GetDMXPersonalityDescription(const RDMHeader *header,
                                              const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint8_t index = param_data[0];
  if (index == 0 || index > g_responder.def->personality_count) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  if (!g_responder.def->personalities) {
    return RDMResponder_BuildNack(header, NR_HARDWARE_FAULT);
  }

  ReturnUnlessUnicast(header);
  const PersonalityDefinition *personality =
      &g_responder.def->personalities[index - 1];


  unsigned int offset = sizeof(RDMHeader);
  g_rdm_buffer[offset++] = index;
  g_rdm_buffer[offset++] = ShortMSB(personality->dmx_footprint);
  g_rdm_buffer[offset++] = ShortLSB(personality->dmx_footprint);
  offset += RDMUtil_StringCopy((char*) g_rdm_buffer + offset,
                               RDM_DEFAULT_STRING_SIZE,
                               personality->description,
                               RDM_DEFAULT_STRING_SIZE);

  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id),
                           offset - sizeof(RDMHeader));
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GetDMXStartAddress(const RDMHeader *header,
                                    UNUSED const uint8_t *param_data) {
  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  ReturnUnlessUnicast(header);

  unsigned int offset = sizeof(RDMHeader);
  g_rdm_buffer[offset++] = ShortMSB(g_responder.dmx_start_address);
  g_rdm_buffer[offset++] = ShortLSB(g_responder.dmx_start_address);
  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id),
                           offset - sizeof(RDMHeader));
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_SetDMXStartAddress(const RDMHeader *header,
                                    const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint16_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint16_t address = JoinShort(param_data[0], param_data[1]);
  if (address == 0 || address > MAX_DMX_START_ADDRESS) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_responder.dmx_start_address = address;
  return RDMResponder_BuildSetAck(header);
}

int RDMResponder_GetSensorDefinition(const RDMHeader *header,
                                     const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  ReturnUnlessUnicast(header);

  uint8_t sensor_index = param_data[0];

  if (sensor_index >= g_responder.def->sensor_count) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  const SensorDefinition *sensor_ptr = &g_responder.def->sensors[sensor_index];

  struct sensor_definition_s {
    uint8_t index;
    uint8_t type;
    uint8_t unit;
    uint8_t prefix;
    int16_t range_minimum_value;
    int16_t range_maximum_value;
    int16_t normal_minimum_value;
    int16_t normal_maximum_value;
    uint8_t recorded_value_support;
    char description[RDM_DEFAULT_STRING_SIZE];
  } __attribute__((packed));

  struct sensor_definition_s sensor_def = {
    .index = sensor_index,
    .type = sensor_ptr->type,
    .unit = sensor_ptr->unit,
    .prefix = sensor_ptr->prefix,
    .range_minimum_value = htons(sensor_ptr->range_minimum_value),
    .range_maximum_value = htons(sensor_ptr->range_maximum_value),
    .normal_minimum_value = htons(sensor_ptr->normal_minimum_value),
    .normal_maximum_value = htons(sensor_ptr->normal_maximum_value),
    .recorded_value_support = sensor_ptr->recorded_value_support
  };

  // copy description
  unsigned int str_len = RDMUtil_StringCopy(
      sensor_def.description, RDM_DEFAULT_STRING_SIZE, sensor_ptr->description,
      RDM_DEFAULT_STRING_SIZE);

  unsigned int param_data_size = sizeof(sensor_def) - RDM_DEFAULT_STRING_SIZE +
                                 str_len;
  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id), param_data_size);
  memcpy(g_rdm_buffer + sizeof(RDMHeader),
         (const uint8_t*) &sensor_def, param_data_size);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_GetSensorValue(const RDMHeader *header,
                                const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }
  ReturnUnlessUnicast(header);

  uint8_t sensor_index = param_data[0];

  if (sensor_index >= g_responder.def->sensor_count) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  SensorData *sensor_ptr = &g_responder.sensors[sensor_index];

  if (sensor_ptr->should_nack) {
    return RDMResponder_BuildNack(header, sensor_ptr->nack_reason);
  }

  BuildSensorValueResponse(sensor_index, sensor_ptr);
  RDMResponder_BuildHeader(header, ACK, GET_COMMAND_RESPONSE,
                           ntohs(header->param_id),
                           SENSOR_VALUE_PARAM_DATA_LENGTH);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_SetSensorValue(const RDMHeader *header,
                                const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint8_t sensor_index = param_data[0];
  if (sensor_index < g_responder.def->sensor_count) {
    ResetSensor(sensor_index);
  } else if (sensor_index == ALL_SENSORS) {
    unsigned int i = 0;
    for (; i < g_responder.def->sensor_count; i++) {
      ResetSensor(i);
    }
  } else {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  ReturnUnlessUnicast(header);

  unsigned int offset = sizeof(RDMHeader);
  if (sensor_index == ALL_SENSORS) {
    memset(g_rdm_buffer + offset, 0, SENSOR_VALUE_PARAM_DATA_LENGTH);
  } else {
    BuildSensorValueResponse(sensor_index, &g_responder.sensors[sensor_index]);
  }

  RDMResponder_BuildHeader(header, ACK, SET_COMMAND_RESPONSE,
                           ntohs(header->param_id),
                           SENSOR_VALUE_PARAM_DATA_LENGTH);
  return RDMUtil_AppendChecksum(g_rdm_buffer);
}

int RDMResponder_SetRecordSensor(const RDMHeader *header,
                                 const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint8_t sensor_index = param_data[0];
  if (sensor_index < g_responder.def->sensor_count) {
    if (g_responder.def->sensors[sensor_index].recorded_value_support &
        SENSOR_SUPPORTS_RECORDING_MASK) {
      RecordSensor(sensor_index);
      return RDMResponder_BuildSetAck(header);
    } else {
      return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
    }
  } else if (sensor_index == ALL_SENSORS) {
    unsigned int i = 0;
    for (; i < g_responder.def->sensor_count; i++) {
      RecordSensor(i);
    }
    return RDMResponder_BuildSetAck(header);
  } else {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }
}

int RDMResponder_GetIdentifyDevice(const RDMHeader *header,
                                   UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetBool(header, g_responder.identify_on);
}

int RDMResponder_SetIdentifyDevice(const RDMHeader *header,
                                   const uint8_t *param_data) {
  return RDMResponder_GenericSetBool(header, param_data,
                                     &g_responder.identify_on);
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
