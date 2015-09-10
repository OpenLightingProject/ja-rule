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
 * rdm_handler.c
 * Copyright (C) 2015 Simon Newton
 */
#include "rdm_handler.h"

#include <stdlib.h>
#include <string.h>

#include "app_pipeline.h"
#include "constants.h"
#include "iovec.h"
#include "macros.h"
#include "rdm_buffer.h"
#include "rdm_frame.h"
#include "rdm_util.h"
#include "syslog.h"
#include "utils.h"

enum { MAX_RDM_MODELS = 6 };

static ModelEntry g_models[MAX_RDM_MODELS];

typedef struct {
  uint16_t default_model;
  ModelEntry *active_model;
  RDMHandlerSendCallback send_callback;
} RDMHandlerState;

static RDMHandlerState g_rdm_handler;

static int GetSetModelId(const RDMHeader *header,
                         const uint8_t *param_data) {
  uint8_t our_uid[UID_LENGTH];
  RDMHandler_GetUID(our_uid);

  if (!RDMUtil_RequiresAction(our_uid, header->dest_uid)) {
    return RDM_RESPONDER_NO_RESPONSE;
  }

  uint16_t sub_device = ntohs(header->sub_device);
  // No subdevice support for now.
  if (sub_device != SUBDEVICE_ROOT && sub_device != SUBDEVICE_ALL) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  } else if (sub_device == SUBDEVICE_ALL &&
             header->command_class == GET_COMMAND) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  if (header->command_class == GET_COMMAND) {
    if (header->param_data_length) {
      return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
    }

    if (RDMUtil_UIDCompare(our_uid, header->dest_uid)) {
      return RDM_RESPONDER_NO_RESPONSE;
    }

    uint16_t model_id = NULL_MODEL_ID;
    if (g_rdm_handler.active_model) {
      model_id = g_rdm_handler.active_model->model_id;
    }

    uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
    ptr = PushUInt16(ptr, model_id);
    return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
  } else if (header->command_class == SET_COMMAND) {
    if (header->param_data_length != sizeof(uint16_t)) {
      return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
    }

    // take action
    uint16_t new_model = JoinShort(param_data[0], param_data[1]);
    bool ok = RDMHandler_SetActiveModel(new_model);

    if (RDMUtil_UIDCompare(our_uid, header->dest_uid)) {
      return RDM_RESPONDER_NO_RESPONSE;
    }

    if (!ok) {
      return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
    }

    return RDMResponder_BuildSetAck(header);
  }
  return RDM_RESPONDER_NO_RESPONSE;
}

static int GetModelList(const RDMHeader *header) {
  uint8_t our_uid[UID_LENGTH];
  RDMHandler_GetUID(our_uid);

  if (RDMUtil_UIDCompare(our_uid, header->dest_uid)) {
    return RDM_RESPONDER_NO_RESPONSE;
  }

  uint16_t sub_device = ntohs(header->sub_device);
  // No subdevice support for now.
  if (sub_device != SUBDEVICE_ROOT) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  if (header->command_class != GET_COMMAND) {
    return RDMResponder_BuildNack(header, NR_UNSUPPORTED_COMMAND_CLASS);
  }

  if (header->param_data_length) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  unsigned int i = 0u;
  for (; i < MAX_RDM_MODELS; i++) {
    if (g_models[i].model_id != NULL_MODEL_ID) {
      ptr = PushUInt16(ptr, g_models[i].model_id);
    }
  }

  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

// Public Functions
// ----------------------------------------------------------------------------
void RDMHandler_Initialize(const RDMHandlerSettings *settings) {
  g_rdm_handler.default_model = settings->default_model;
  g_rdm_handler.active_model = NULL;
  g_rdm_handler.send_callback = settings->send_callback;

  unsigned int i = 0u;
  for (; i < MAX_RDM_MODELS; i++) {
    g_models[i].model_id = NULL_MODEL_ID;
  }
}

bool RDMHandler_AddModel(const ModelEntry *entry) {
  unsigned int i = 0u;
  for (; i < MAX_RDM_MODELS; i++) {
    if (g_models[i].model_id == entry->model_id) {
      return false;
    }

    if (g_models[i].model_id == NULL_MODEL_ID) {
      g_models[i].model_id = entry->model_id;
      g_models[i].activate_fn = entry->activate_fn;
      g_models[i].deactivate_fn = entry->deactivate_fn;
      g_models[i].ioctl_fn = entry->ioctl_fn;
      g_models[i].request_fn = entry->request_fn;
      g_models[i].tasks_fn = entry->tasks_fn;
      if (entry->model_id == g_rdm_handler.default_model) {
        g_rdm_handler.active_model = &g_models[i];
        g_rdm_handler.active_model->activate_fn();
      }
      return true;
    }
  }
  return false;
}

bool RDMHandler_SetActiveModel(uint16_t model_id) {
  if (g_rdm_handler.active_model &&
      g_rdm_handler.active_model->model_id == model_id) {
    return true;
  }

  if (model_id == NULL_MODEL_ID) {
    if (g_rdm_handler.active_model) {
      g_rdm_handler.active_model->deactivate_fn();
    }
    g_rdm_handler.active_model = NULL;
    return true;
  }

  unsigned int i = 0u;
  for (; i < MAX_RDM_MODELS; i++) {
    if (g_models[i].model_id != NULL_MODEL_ID &&
        g_models[i].model_id == model_id) {
      if (g_rdm_handler.active_model) {
        g_rdm_handler.active_model->deactivate_fn();
      }
      g_rdm_handler.active_model = &g_models[i];
      g_rdm_handler.active_model->activate_fn();
      return true;
    }
  }
  return false;
}

uint16_t RDMHandler_ActiveModel() {
  return g_rdm_handler.active_model ? g_rdm_handler.active_model->model_id :
      NULL_MODEL_ID;
}

void RDMHandler_HandleRequest(const RDMHeader *header,
                              const uint8_t *param_data) {
  // We need to intercept calls to the SET_MODEL_ID pid, and use them to change
  // the active model.
  int response_size = RDM_RESPONDER_NO_RESPONSE;

  if (ntohs(header->param_id) == PID_DEVICE_MODEL) {
    response_size = GetSetModelId(header, param_data);
  } else if (ntohs(header->param_id) == PID_DEVICE_MODEL_LIST) {
    response_size = GetModelList(header);
  } else {
    if (!g_rdm_handler.active_model) {
      return;
    }

    response_size = g_rdm_handler.active_model->request_fn(header, param_data);
  }

  if (response_size) {
    IOVec iov;
    iov.base = g_rdm_buffer;
    iov.length = abs(response_size);

#ifdef PIPELINE_RDMRESPONDER_SEND
    PIPELINE_RDMRESPONDER_SEND(response_size < 0 ? false : true, &iov, 1u);
#else
    g_rdm_handler.send_callback(response_size < 0 ? false : true, &iov, 1u);
#endif
  }
}

void RDMHandler_GetUID(uint8_t *uid) {
  if (g_rdm_handler.active_model) {
    g_rdm_handler.active_model->ioctl_fn(IOCTL_GET_UID, uid, UID_LENGTH);
  } else {
    memset(uid, 0, UID_LENGTH);
  }
}

void RDMHandler_Tasks() {
  if (g_rdm_handler.active_model) {
    g_rdm_handler.active_model->tasks_fn();
  }
}
