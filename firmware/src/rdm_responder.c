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
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "iovec.h"
#include "rdm_frame.h"
#include "system_pipeline.h"
#include "utils.h"

// Various constants
#define FIVE5_CONSTANT 0x55
#define AA_CONSTANT 0xaa
#define FE_CONSTANT 0xfe
#define DUB_RESPONSE_LENGTH 24
#define CHECKSUM_SIZE 2
#define MODEL_ID 0x0100
#define RDM_VERSION 0x0100

static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule Responder";
static const char MANUFACTURER_LABEL[] = "Open Lighting Project";
static const char SOFTWARE_LABEL[] = "Alpha";

typedef struct {
  RDMResponderSendCallback send_callback;
  uint8_t uid[UID_LENGTH];
  uint8_t queued_message_count;
  bool is_muted;
  uint16_t dmx_start_address;

  // Used to construct responses so we avoid using stack space.
  uint8_t param_data[MAX_PARAM_DATA_SIZE];
} RDMResponderData;

RDMResponderData g_rdm_responder;

/*
 * @brief Check if the UID matches our UID.
 */
static inline bool RequiresResponse(const RDMHeader *header) {
  // Check if UID is for us
  return memcmp(g_rdm_responder.uid, header->dest_uid, UID_LENGTH) == 0;
}

/*
 * @brief uid1 < uid2
 */
static inline int UIDCompare(const uint8_t *uid1, const uint8_t *uid2) {
  return memcmp(uid1, uid2, UID_LENGTH);
}

/*
 * @brief Generate the checksum for a RDM frame.
 */
static void Checksum(const IOVec* iov, unsigned int iov_count,
                     uint8_t checksum_data[CHECKSUM_SIZE]) {
  uint16_t checksum = 0;
  unsigned int i, j;
  for (i = 0; i != iov_count; i++) {
    for (j = 0; j != iov[i].length; j++) {
      checksum += ((uint8_t*) iov[i].base)[j];
    }
  }
  checksum_data[0] = ShortMSB(checksum);
  checksum_data[1] = ShortLSB(checksum);
}

/*
 * @brief Send a DUB response if our UID falls within the range.
 */
static void SendDUBResponseIfRequired(const uint8_t *param_data,
                                      unsigned int param_data_length) {
  if (param_data_length != 2 * UID_LENGTH) {
    return;
  }

  if (!(UIDCompare(param_data, g_rdm_responder.uid) <= 0 &&
        UIDCompare(g_rdm_responder.uid, param_data + UID_LENGTH) <= 0)) {
    return;
  }

  uint8_t response[DUB_RESPONSE_LENGTH];
  memset(response, FE_CONSTANT, 7);
  response[7] = AA_CONSTANT;
  response[8] = g_rdm_responder.uid[0] | AA_CONSTANT;
  response[9] = g_rdm_responder.uid[0] | FIVE5_CONSTANT;
  response[10] = g_rdm_responder.uid[1] | AA_CONSTANT;
  response[11] = g_rdm_responder.uid[1] | FIVE5_CONSTANT;

  response[12] = g_rdm_responder.uid[2] | AA_CONSTANT;
  response[13] = g_rdm_responder.uid[2] | FIVE5_CONSTANT;
  response[14] = g_rdm_responder.uid[3] | AA_CONSTANT;
  response[15] = g_rdm_responder.uid[3] | FIVE5_CONSTANT;
  response[16] = g_rdm_responder.uid[4] | AA_CONSTANT;
  response[17] = g_rdm_responder.uid[4] | FIVE5_CONSTANT;
  response[18] = g_rdm_responder.uid[5] | AA_CONSTANT;
  response[19] = g_rdm_responder.uid[5] | FIVE5_CONSTANT;

  uint16_t checksum = 0;
  unsigned int i;
  for (i = 8; i < 20; i++) {
    checksum += response[i];
  }

  response[20] = ShortMSB(checksum) | AA_CONSTANT;
  response[21] = ShortMSB(checksum) | FIVE5_CONSTANT;
  response[22] = ShortLSB(checksum) | AA_CONSTANT;
  response[23] = ShortLSB(checksum) | FIVE5_CONSTANT;

  IOVec iov;
  iov.base = response;
  iov.length = DUB_RESPONSE_LENGTH;

#ifdef PIPELINE_RDMRESPONDER_SEND
  PIPELINE_RDMRESPONDER_SEND(false, &iov, 1);
#else
  g_rdm_responder.send_callback(false, &iov, 1);
#endif
}

/*
 * @brief Send a RDM response, if the request was not broadcast.
 */
static void RespondIfRequired(const RDMHeader *incoming_header,
                              RDMResponseType response_type,
                              RDMCommandClass command_class,
                              RDMPid pid,
                              const uint8_t *param_data,
                              unsigned int param_data_length) {
  if (!RequiresResponse(incoming_header)) {
    return;
  }

  uint8_t start_code = RDM_START_CODE;

  RDMHeader outgoing_header;
  outgoing_header.sub_start_code = SUB_START_CODE;
  outgoing_header.message_length = (
      sizeof(start_code) + sizeof(outgoing_header) + param_data_length);
  memcpy(outgoing_header.dest_uid, incoming_header->src_uid, UID_LENGTH);
  memcpy(outgoing_header.src_uid, g_rdm_responder.uid, UID_LENGTH);
  outgoing_header.transaction_number = incoming_header->transaction_number;
  outgoing_header.port_id = response_type;
  outgoing_header.message_count = g_rdm_responder.queued_message_count;
  outgoing_header.sub_device = htons(SUBDEVICE_ROOT);
  outgoing_header.command_class = command_class;
  outgoing_header.param_id = htons(pid);
  outgoing_header.param_data_length = param_data_length;

  IOVec iov[4];

  iov[0].base = &start_code;
  iov[0].length = sizeof(start_code);
  iov[1].base = &outgoing_header;
  iov[1].length = sizeof(outgoing_header);
  uint8_t iovec_length = 2;

  if (param_data && param_data_length) {
    iov[iovec_length].base = param_data;
    iov[iovec_length].length = param_data_length;
    iovec_length++;
  }

  uint8_t checksum_data[CHECKSUM_SIZE];
  Checksum(iov, iovec_length, checksum_data);
  iov[iovec_length].base = &checksum_data;
  iov[iovec_length].length = CHECKSUM_SIZE;
  iovec_length++;

#ifdef PIPELINE_RDMRESPONDER_SEND
  PIPELINE_RDMRESPONDER_SEND(true, iov, iovec_length);
#else
  g_rdm_responder.send_callback(true, iov, iovec_length);
#endif
}

/*
 * @brief NACK the request
 * @pre The command class is GET_COMMAND or SET_COMMAND.
 */
inline void SendNack(const RDMHeader *header,
                     RDMNackReason reason) {
  uint16_t param_data = htons(reason);
  RespondIfRequired(
      header, NACK_REASON,
      (header->command_class == GET_COMMAND ?
       GET_COMMAND_RESPONSE : SET_COMMAND_RESPONSE),
      ntohs(header->param_id),
      (uint8_t*) &param_data, sizeof(param_data));
}

// PID Handlers
// ----------------------------------------------------------------------------

static void Mute(const RDMHeader *header,
                 unsigned int param_data_length) {
  if (param_data_length != 0) {
    return;
  }
  g_rdm_responder.is_muted = true;

  RespondIfRequired(header, ACK, DISCOVER_COMMAND_RESPONSE, PID_DISC_MUTE,
                    NULL, 0);
}

static void UnMute(const RDMHeader *header,
                   unsigned int param_data_length) {
  if (param_data_length != 0) {
    return;
  }
  g_rdm_responder.is_muted = false;

  RespondIfRequired(header, ACK, DISCOVER_COMMAND_RESPONSE, PID_DISC_UN_MUTE,
                    NULL, 0);
}

static void GetDeviceInfo(const RDMHeader *header,
                          unsigned int param_data_length) {
  if (param_data_length != 0) {
    SendNack(header, NR_FORMAT_ERROR);
    return;
  }

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
  }__attribute__((packed));

  struct device_info_s *device_info =
      (struct device_info_s*) g_rdm_responder.param_data;
  device_info->rdm_version = htons(RDM_VERSION);
  device_info->model = htons(MODEL_ID);
  device_info->product_category = htons(PRODUCT_CATEGORY_TEST_EQUIPMENT);
  device_info->software_version = htonl(0);
  device_info->dmx_footprint = htons(0);
  device_info->current_personality = 0;
  device_info->personality_count = 0;
  device_info->dmx_start_address = htons(g_rdm_responder.dmx_start_address);
  device_info->sub_device_count = htons(0);
  device_info->sensor_count = 0;

  RespondIfRequired(header, ACK, GET_COMMAND_RESPONSE, PID_DEVICE_INFO,
                    g_rdm_responder.param_data, sizeof(struct device_info_s));
}

static void GetSupportedParameters(const RDMHeader *header,
                                   unsigned int param_data_length) {
  if (param_data_length != 0) {
    SendNack(header, NR_FORMAT_ERROR);
    return;
  }

  uint16_t *pids = (uint16_t*) g_rdm_responder.param_data;
  unsigned int i = 0;
  pids[i++] = htons(PID_DEVICE_MODEL_DESCRIPTION);
  pids[i++] = htons(PID_MANUFACTURER_LABEL);

  RespondIfRequired(header, ACK, GET_COMMAND_RESPONSE,
                    PID_SUPPORTED_PARAMETERS,
                    g_rdm_responder.param_data, i * sizeof(pids[0]));
}

/*
 * @brief Reply with a string
 * @param header
 * @param param_data_length
 * @param reply_string The string to reply with
 * @param string_size The length of the string, including the terminating NULL.
 */
static void GenericReturnString(const RDMHeader *header,
                                unsigned int param_data_length,
                                const char *reply_string,
                                unsigned int string_size) {
  if (param_data_length != 0) {
    SendNack(header, NR_FORMAT_ERROR);
    return;
  }

  RespondIfRequired(header, ACK, GET_COMMAND_RESPONSE,
                    ntohs(header->param_id),
                    (const uint8_t*) reply_string,
                    string_size - 1);
}

// Public Functions
// ----------------------------------------------------------------------------
void RDMResponder_Initialize(const uint8_t uid[UID_LENGTH],
                             RDMResponderSendCallback send_callback) {
  g_rdm_responder.send_callback = send_callback;
  memcpy(g_rdm_responder.uid, uid, UID_LENGTH);
  g_rdm_responder.queued_message_count = 0;
  g_rdm_responder.is_muted = false;
  g_rdm_responder.dmx_start_address = 0xffff;
}

bool RDMResponder_UIDRequiresAction(const uint8_t uid[UID_LENGTH]) {
  if (memcmp(g_rdm_responder.uid, uid, UID_LENGTH) == 0) {
    return true;
  }

  if (uid[2] != 0xff || uid[3] != 0xff || uid[4] != 0xff || uid[5] != 0xff) {
    // definitely not a broadcast
    return false;
  }

  return (uid[0] == g_rdm_responder.uid[0] && uid[1] == g_rdm_responder.uid[1])
          ||
         (uid[0] == 0xff && uid[1] == 0xff);
}

void RDMResponder_HandleRequest(const RDMHeader *header,
                                const uint8_t *param_data,
                                unsigned int param_data_length) {
  uint16_t sub_device = ntohs(header->sub_device);

  // No subdevice support for now.
  if (sub_device != SUBDEVICE_ROOT && sub_device != SUBDEVICE_ALL) {
    SendNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
    return;
  }

  uint16_t param_id = ntohs(header->param_id);

  if (header->command_class == DISCOVER_COMMAND) {
    switch (param_id) {
      case PID_DISC_UNIQUE_BRANCH:
        SendDUBResponseIfRequired(param_data, param_data_length);
        {}
        break;
      case PID_DISC_MUTE:
        Mute(header, param_data_length);
        break;
      case PID_DISC_UN_MUTE:
        UnMute(header, param_data_length);
        break;
      default:
        {}
    }
    return;
  }

  bool is_get = header->command_class == GET_COMMAND;

  switch (param_id) {
    case PID_DEVICE_INFO:
      if (is_get) {
        GetDeviceInfo(header, param_data_length);
      } else {
        SendNack(header, NR_UNSUPPORTED_COMMAND_CLASS);
      }
      break;
    case PID_SUPPORTED_PARAMETERS:
      if (is_get) {
        GetSupportedParameters(header, param_data_length);
      } else {
        SendNack(header, NR_UNSUPPORTED_COMMAND_CLASS);
      }
      break;
    case PID_DEVICE_MODEL_DESCRIPTION:
      if (is_get) {
        GenericReturnString(header, param_data_length, DEVICE_MODEL_DESCRIPTION,
                            sizeof(DEVICE_MODEL_DESCRIPTION));
      } else {
        SendNack(header, NR_UNSUPPORTED_COMMAND_CLASS);
      }
      break;
    case PID_MANUFACTURER_LABEL:
      if (is_get) {
        GenericReturnString(header, param_data_length, MANUFACTURER_LABEL,
                            sizeof(MANUFACTURER_LABEL));
      } else {
        SendNack(header, NR_UNSUPPORTED_COMMAND_CLASS);
      }
      break;
    case PID_SOFTWARE_VERSION_LABEL:
      if (is_get) {
        GenericReturnString(header, param_data_length, SOFTWARE_LABEL,
                            sizeof(SOFTWARE_LABEL));
      } else {
        SendNack(header, NR_UNSUPPORTED_COMMAND_CLASS);
      }
      break;
    default:
      SendNack(header, NR_UNKNOWN_PID);
  }
}

bool RDMResponder_IsMuted() {
  return g_rdm_responder.is_muted;
}
