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

#include "constants.h"
#include "iovec.h"
#include "rdm_frame.h"
#include "rdm_util.h"
#include "rdm_buffer.h"
#include "system_pipeline.h"

#define MAX_RDM_MODELS 4

static ModelEntry g_models[MAX_RDM_MODELS];

typedef struct {
  uint16_t default_model;
  ModelEntry *active_model;
  RDMHandlerSendCallback send_callback;
} RDMHandlerState;

static RDMHandlerState g_rdm_handler;

// Public Functions
// ----------------------------------------------------------------------------
void RDMHandler_Initialize(const RDMHandlerSettings *settings) {
  g_rdm_handler.default_model = settings->default_model;
  g_rdm_handler.active_model = NULL;
  g_rdm_handler.send_callback = settings->send_callback;

  unsigned int i = 0;
  for (; i < MAX_RDM_MODELS; i++) {
    g_models[i].model_id = NULL_MODEL;
  }
}

bool RDMHandler_AddModel(const ModelEntry *entry) {
  unsigned int i = 0;
  for (; i < MAX_RDM_MODELS; i++) {
    if (g_models[i].model_id == entry->model_id) {
      return false;
    }

    if (g_models[i].model_id == NULL_MODEL) {
      g_models[i].model_id = entry->model_id;
      g_models[i].activate_fn = entry->activate_fn;
      g_models[i].deactivate_fn = entry->deactivate_fn;
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

  if (model_id == NULL_MODEL) {
    if (g_rdm_handler.active_model) {
      g_rdm_handler.active_model->deactivate_fn();
    }
    g_rdm_handler.active_model = NULL;
    return true;
  }

  unsigned int i = 0;
  for (; i < MAX_RDM_MODELS; i++) {
    if (g_models[i].model_id != NULL_MODEL &&
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

void RDMHandler_HandleRequest(const RDMHeader *header,
                              const uint8_t *param_data) {
  // TODO(simon):
  // We need to intercept calls to the SET_MODEL_ID pid, and use them to change
  // the active model.

  if (!g_rdm_handler.active_model) {
    return;
  }

  int response_size = g_rdm_handler.active_model->request_fn(header,
                                                             param_data);
  if (response_size) {
    IOVec iov;
    iov.base = g_rdm_buffer;
    iov.length = abs(response_size);

#ifdef PIPELINE_RDMRESPONDER_SEND
    PIPELINE_RDMRESPONDER_SEND(response_size < 0 ? false : true, &iov, 1);
#else
    g_rdm_handler.send_callback(response_size < 0 ? false : true, &iov, 1);
#endif
  }
}

void RDMHandler_GetUID(uint8_t *uid) {
  if (g_rdm_handler.active_model) {
    g_rdm_handler.active_model->tasks_fn(uid, UID_LENGTH);
  } else {
    memset(uid, 0, UID_LENGTH);
  }
}

void RDMHandler_Tasks() {
  if (g_rdm_handler.active_model) {
    g_rdm_handler.active_model->tasks_fn();
  }
}
