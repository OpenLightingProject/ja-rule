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

#include "coarse_timer.h"
#include "constants.h"
#include "macros.h"
#include "rdm_frame.h"
#include "rdm_buffer.h"
#include "rdm_responder.h"
#include "rdm_util.h"
#include "utils.h"

#include <syslog.h>

#include <system_config.h>

// Various constants
enum { NUMBER_OF_SUB_DEVICES = 4 };
enum { NUMBER_OF_SCENES = 3 };
enum { NUMBER_OF_LOCK_STATES = 3 };
enum { NUMBER_OF_CURVES = 4 };
enum { NUMBER_OF_OUTPUT_RESPONSE_TIMES = 2 };
enum { NUMBER_OF_MODULATION_FREQUENCIES = 4 };
enum { NUMBER_OF_SELF_TESTS = 2 };
enum { STATUS_MESSAGE_QUEUE_SIZE = 4 };
enum { PERSONALITY_COUNT = 1 };
enum { SOFTWARE_VERSION = 0x00000000 };
static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule Dimmer Device";
static const char SOFTWARE_LABEL[] = "Alpha";
static const char DEFAULT_DEVICE_LABEL[] = "Ja Rule";
static const char PERSONALITY_DESCRIPTION[] = "Dimmer";
static const uint8_t STATUS_TYPE_MASK = 0xf;
static const uint16_t INITIAL_START_ADDRESSS = 1u;
static const uint32_t STATUS_MESSAGE_TRIGGER_INTERVAL = 300000;  // 30s

static const char LOCK_STATE_DESCRIPTION_UNLOCKED[] = "Unlocked";
static const char LOCK_STATE_DESCRIPTION_SUBDEVICES_LOCKED[] =
    "Subdevices locked";
static const char LOCK_STATE_DESCRIPTION_ALL_LOCKED[] =
    "Root & subdevices locked";

static const char CURVE_DESCRIPTION1[] = "Linear";
static const char CURVE_DESCRIPTION2[] = "Modified Linear";
static const char CURVE_DESCRIPTION3[] = "Square";
static const char CURVE_DESCRIPTION4[] = "Modified Square";

static const char SELF_TEST_DESCRIPTION1[] = "Quick test";
static const char SELF_TEST_DESCRIPTION2[] = "Extensive test";

static const char OUTPUT_RESPONSE_DESCRIPTION1[] = "Fast";
static const char OUTPUT_RESPONSE_DESCRIPTION2[] = "Slow";

static const char MODULATION_FREQUENCY_DESCRIPTION1[] = "50Hz";
static const char MODULATION_FREQUENCY_DESCRIPTION2[] = "60Hz";
static const char MODULATION_FREQUENCY_DESCRIPTION3[] = "1kHz";
static const char MODULATION_FREQUENCY_DESCRIPTION4[] = "2kHz";

static const char STS_OLP_TESTING_DESCRIPTION[] = "Counter cycle %d.%d";

enum {
  LOCK_STATE_UNLOCKED = 0x0000,
  LOCK_STATE_SUBDEVICES_LOCKED = 0x0001,
  LOCK_STATE_ALL_LOCKED = 0x0002,
};

typedef struct {
  uint32_t frequency;
  const char *description;
} ModulationFrequency;

typedef struct {
  uint16_t up_fade_time;
  uint16_t down_fade_time;
  uint16_t wait_time;
  uint8_t programmed_state;
} Scene;

typedef struct {
  uint16_t sub_device;
  uint16_t message_id;
  uint16_t data_value1;
  uint16_t data_value2;
  RDMStatusType status_type;
  bool is_active;
} StatusMessage;

typedef struct {
  uint32_t duration;
  const char *description;
} SelfTest;

typedef struct {
  /*
   * @brief Since 0 means 'off', scene numbers are indexed from 1.
   *
   * Remember this when using the array.
   */
  Scene scenes[NUMBER_OF_SCENES];
  CoarseTimer_Value status_message_timer;
  CoarseTimer_Value self_test_timer;
  StatusMessage status_message;

  uint16_t playback_mode;
  uint16_t startup_scene;
  uint16_t startup_delay;
  uint16_t startup_hold;
  uint16_t fail_scene;
  uint16_t fail_loss_of_signal_delay;
  uint16_t fail_hold_time;
  uint16_t pin_code;

  uint8_t fail_level;
  uint8_t startup_level;
  uint8_t playback_level;
  uint8_t lock_state;
  uint8_t merge_mode;

  bool power_on_self_test;
  uint8_t running_self_test;
} RootDevice;

typedef struct {
  RDMResponder responder;
  StatusMessage status_message;

  uint16_t index;
  uint16_t min_level_increasing;
  uint16_t min_level_decreasing;
  uint16_t max_level;
  uint8_t on_below_min;
  uint8_t identify_mode;
  uint8_t burn_in;
  uint8_t curve;
  uint8_t output_response_time;
  uint8_t modulation_frequency;
  RDMStatusType sd_report_threshold;
} DimmerSubDevice;

typedef struct {
  StatusMessage last[STATUS_MESSAGE_QUEUE_SIZE];
  uint8_t count;
} StatusMessages;

static DimmerSubDevice g_subdevices[NUMBER_OF_SUB_DEVICES];

static const char* LOCK_STATES[NUMBER_OF_LOCK_STATES] = {
  LOCK_STATE_DESCRIPTION_UNLOCKED,
  LOCK_STATE_DESCRIPTION_SUBDEVICES_LOCKED,
  LOCK_STATE_DESCRIPTION_ALL_LOCKED
};

static const char* DIMMER_CURVES[NUMBER_OF_CURVES] = {
  CURVE_DESCRIPTION1,
  CURVE_DESCRIPTION2,
  CURVE_DESCRIPTION3,
  CURVE_DESCRIPTION4,
};

static const char* OUTPUT_RESPONSE_TIMES[NUMBER_OF_OUTPUT_RESPONSE_TIMES] = {
  OUTPUT_RESPONSE_DESCRIPTION1,
  OUTPUT_RESPONSE_DESCRIPTION2,
};

static const ModulationFrequency
MODULATION_FREQUENCY[NUMBER_OF_MODULATION_FREQUENCIES] = {
  {
    .frequency = 50u,
    .description = MODULATION_FREQUENCY_DESCRIPTION1,
  },
  {
    .frequency = 60u,
    .description = MODULATION_FREQUENCY_DESCRIPTION2,
  },
  {
    .frequency = 1000u,
    .description = MODULATION_FREQUENCY_DESCRIPTION3,
  },
  {
    .frequency = 2000u,
    .description = MODULATION_FREQUENCY_DESCRIPTION4,
  },
};

static const SelfTest SELF_TESTS[NUMBER_OF_SELF_TESTS] = {
  {
    .duration = 50000,  // 5s
    .description = SELF_TEST_DESCRIPTION1,
  },
  {
    .duration = 200000,  // 20s
    .description = SELF_TEST_DESCRIPTION2,
  },
};

static const ResponderDefinition ROOT_RESPONDER_DEFINITION;
static const ResponderDefinition SUBDEVICE_RESPONDER_DEFINITION;

static StatusMessages g_status_messages;


static RootDevice g_root_device;
static DimmerSubDevice *g_active_device = NULL;

// Helper functions
// ----------------------------------------------------------------------------

/*
 * @brief Set a block address for all the sub devices.
 * @param start_address the new start address
 * @returns true if the start address of all subdevices changed, false if the
 *   footprint of the sub devices would exceed the last slot (512).
 */
bool ResetToBlockAddress(uint16_t start_address) {
  unsigned int footprint = 0u;
  unsigned int i = 0u;
  for (; i < NUMBER_OF_SUB_DEVICES; i++) {
    RDMResponder *responder = &g_subdevices[i].responder;
    footprint +=
        responder->def->personalities[responder->current_personality - 1u]
            .slot_count;
  }

  if ((uint16_t) (MAX_DMX_START_ADDRESS - start_address + 1u) < footprint) {
    return false;
  }

  for (i = 0u; i < NUMBER_OF_SUB_DEVICES; i++) {
    RDMResponder *responder = &g_subdevices[i].responder;
    responder->dmx_start_address = start_address;
    const PersonalityDefinition *personality =
        &responder->def->personalities[responder->current_personality - 1u];
    start_address += personality->slot_count;
  }
  return true;
}

uint8_t *AddStatusMessageToResponse(uint8_t *ptr,
                                    const StatusMessage *message) {
  ptr = PushUInt16(ptr, message->sub_device);
  *ptr++ = message->status_type;
  ptr = PushUInt16(ptr, message->message_id);
  ptr = PushUInt16(ptr, message->data_value1);
  ptr = PushUInt16(ptr, message->data_value2);
  return ptr;
}

void QueueStatusMessage(StatusMessage *message,
                        uint16_t sub_device_index,
                        RDMStatusType status_type,
                        RDMStatusMessageId status_id,
                        uint16_t data_value1,
                        uint16_t data_value2) {
  message->is_active = true;
  message->sub_device = sub_device_index;
  message->status_type = status_type;
  message->message_id = status_id;
  message->data_value1 = data_value1;
  message->data_value2 = data_value2;
}

void QueueSubDeviceStatusMessage(DimmerSubDevice *device,
                                 RDMStatusType status_type,
                                 RDMStatusMessageId status_id,
                                 uint16_t data_value1,
                                 uint16_t data_value2) {
  if (device->sd_report_threshold == STATUS_NONE ||
      (status_type & STATUS_TYPE_MASK) < device->sd_report_threshold) {
    return;
  }

  QueueStatusMessage(&device->status_message, device->index,
                     status_type, status_id, data_value1, data_value2);
}

/*
 * @brief Dequeue a status message if it's at or above the threshold.
 * @pre There is at least one message slot available in g_status_messages.
 * @returns true if the message was added, false otherwise.
 */
bool MaybeDequeueStatusMessage(StatusMessage *message, uint8_t threshold) {
  if (!message->is_active ||
      threshold < (message->status_type & STATUS_TYPE_MASK)) {
    return false;
  }

  message->is_active = false;
  g_status_messages.last[g_status_messages.count] = *message;
  return true;
}

// Root PID Handlers
// ----------------------------------------------------------------------------
int DimmerModel_GetStatusMessages(const RDMHeader *header,
                                  UNUSED const uint8_t *param_data) {
  uint8_t threshold = param_data[0];

  if (threshold > STATUS_ERROR) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);

  if (threshold == STATUS_GET_LAST_MESSAGE) {
    // Return the last set of messages.
    unsigned int i = 0u;
    for (; i < g_status_messages.count; i++) {
      ptr = AddStatusMessageToResponse(ptr, &g_status_messages.last[i]);
    }
  } else {
    // Build the list of status messages.
    g_status_messages.count = 0u;

    // Check the root
    if (g_root_device.status_message.is_active &&
        MaybeDequeueStatusMessage(&g_root_device.status_message, threshold)) {
      ptr = AddStatusMessageToResponse(
                ptr, &g_status_messages.last[g_status_messages.count]);
      g_status_messages.count++;
    }

    // Check the sub devices.
    unsigned int i = 0u;
    for (; i < NUMBER_OF_SUB_DEVICES &&
           g_status_messages.count < STATUS_MESSAGE_QUEUE_SIZE; i++) {
      DimmerSubDevice *subdevice = &g_subdevices[i];

      if (MaybeDequeueStatusMessage(&subdevice->status_message, threshold)) {
        ptr = AddStatusMessageToResponse(
                  ptr, &g_status_messages.last[g_status_messages.count]);
        g_status_messages.count++;
      }
    }
  }

  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_GetStatusIdDescription(const RDMHeader *header,
                                       UNUSED const uint8_t *param_data) {
  const uint16_t status_id = ExtractUInt16(param_data);
  if (status_id != STS_OLP_TESTING) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr += RDMUtil_StringCopy((char*) ptr, RDM_DEFAULT_STRING_SIZE,
                            STS_OLP_TESTING_DESCRIPTION,
                            RDM_DEFAULT_STRING_SIZE);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_GetSelfTest(const RDMHeader *header,
                            UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = g_root_device.running_self_test ? true : false;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_PerformSelfTest(const RDMHeader *header,
                                const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint8_t self_test_id = param_data[0];
  // We don't allow cancelling self-tests
  if (self_test_id > NUMBER_OF_SELF_TESTS) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  if (self_test_id == SELF_TEST_OFF) {
    g_root_device.running_self_test = SELF_TEST_OFF;
  } else {
    if (g_root_device.running_self_test) {
      return RDMResponder_BuildNack(header, NR_ACTION_NOT_SUPPORTED);
    }

    g_root_device.running_self_test = self_test_id;
    g_root_device.self_test_timer = CoarseTimer_GetTime();
  }
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetSelfTestDescription(const RDMHeader *header,
                                       UNUSED const uint8_t *param_data) {
  const uint8_t self_test_id = param_data[0];
  if (self_test_id == SELF_TEST_OFF || self_test_id > NUMBER_OF_SELF_TESTS) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = self_test_id;
  ptr += RDMUtil_StringCopy((char*) ptr, RDM_DEFAULT_STRING_SIZE,
                            SELF_TESTS[self_test_id - 1].description,
                            RDM_DEFAULT_STRING_SIZE);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_CapturePreset(const RDMHeader *header,
                              const uint8_t *param_data) {
  if (header->param_data_length != 4 * sizeof(uint16_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t scene_index = ExtractUInt16(&param_data[0]);
  const uint16_t up_fade_time = ExtractUInt16(&param_data[2]);
  const uint16_t down_fade_time = ExtractUInt16(&param_data[4]);
  const uint16_t wait_time = ExtractUInt16(&param_data[6]);

  if (scene_index == 0u || scene_index > NUMBER_OF_SCENES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  Scene *scene = &g_root_device.scenes[scene_index - 1u];

  if (scene->programmed_state == PRESET_PROGRAMMED_READ_ONLY) {
    return RDMResponder_BuildNack(header, NR_WRITE_PROTECT);
  }

  scene->up_fade_time = up_fade_time;
  scene->down_fade_time = down_fade_time;
  scene->wait_time = wait_time;
  scene->programmed_state = PRESET_PROGRAMMED;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetPresetPlayback(const RDMHeader *header,
                                  UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt16(ptr, g_root_device.playback_mode);
  *ptr++ = g_root_device.playback_level;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetPresetPlayback(const RDMHeader *header,
                                  const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint16_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t playback_mode = ExtractUInt16(param_data);
  if (playback_mode > NUMBER_OF_SCENES &&
      playback_mode != PRESET_PLAYBACK_ALL) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_root_device.playback_mode = playback_mode;
  g_root_device.playback_level = param_data[2];

  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetDMXBlockAddress(const RDMHeader *header,
                                   UNUSED const uint8_t *param_data) {
  uint16_t total_footprint = 0u;
  uint16_t expected_start_address = 0u;
  bool is_contiguous = true;
  unsigned int i = 0u;
  for (; i < NUMBER_OF_SUB_DEVICES; i++) {
    RDMResponder *responder = &g_subdevices[i].responder;
    uint16_t sub_device_footprint = responder->def
        ->personalities[responder->current_personality - 1u].slot_count;
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
  if (start_address == 0u || start_address > MAX_DMX_START_ADDRESS) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  bool ok = ResetToBlockAddress(start_address);
  if (ok) {
    return RDMResponder_BuildSetAck(header);
  } else {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }
}

int DimmerModel_GetDMXFailMode(const RDMHeader *header,
                               UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt16(ptr, g_root_device.fail_scene);
  ptr = PushUInt16(ptr, g_root_device.fail_loss_of_signal_delay);
  ptr = PushUInt16(ptr, g_root_device.fail_hold_time);
  *ptr++ = g_root_device.fail_level;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetDMXFailMode(const RDMHeader *header,
                               const uint8_t *param_data) {
  if (header->param_data_length != 3u * sizeof(uint16_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t scene_index = ExtractUInt16(param_data);
  const uint16_t loss_of_signal_delay = ExtractUInt16(&param_data[2]);
  const uint16_t hold_time = ExtractUInt16(&param_data[4]);
  if (scene_index > NUMBER_OF_SCENES && scene_index != PRESET_PLAYBACK_ALL) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_root_device.fail_scene = scene_index;
  g_root_device.fail_loss_of_signal_delay = loss_of_signal_delay;
  g_root_device.fail_hold_time = hold_time;
  g_root_device.fail_level = param_data[6];

  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetDMXStartupMode(const RDMHeader *header,
                                  UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt16(ptr, g_root_device.startup_scene);
  ptr = PushUInt16(ptr, g_root_device.startup_delay);
  ptr = PushUInt16(ptr, g_root_device.startup_hold);
  *ptr++ = g_root_device.startup_level;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetDMXStartupMode(const RDMHeader *header,
                                  const uint8_t *param_data) {
  if (header->param_data_length != 3u * sizeof(uint16_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t scene_index = ExtractUInt16(param_data);
  const uint16_t loss_of_signal_delay = ExtractUInt16(&param_data[2]);
  const uint16_t hold_time = ExtractUInt16(&param_data[4]);
  if (scene_index > NUMBER_OF_SCENES && scene_index != PRESET_PLAYBACK_ALL) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_root_device.startup_scene = scene_index;
  g_root_device.startup_delay = loss_of_signal_delay;
  g_root_device.startup_hold = hold_time;
  g_root_device.startup_level = param_data[6];

  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetPowerOnSelfTest(const RDMHeader *header,
                                   UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetBool(header, g_root_device.power_on_self_test);
}

int DimmerModel_SetPowerOnSelfTest(const RDMHeader *header,
                                   const uint8_t *param_data) {
  return RDMResponder_GenericSetBool(header, param_data,
                                     &g_root_device.power_on_self_test);
}

int DimmerModel_GetLockPin(const RDMHeader *header,
                           UNUSED const uint8_t *param_data) {
  // We allow people to read the PIN
  return RDMResponder_GenericGetUInt16(header, g_root_device.pin_code);
}

int DimmerModel_SetLockPin(const RDMHeader *header,
                           const uint8_t *param_data) {
  if (header->param_data_length != 2u * sizeof(uint16_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t new_pin = ExtractUInt16(param_data);
  const uint16_t old_pin = ExtractUInt16(&param_data[2]);
  if (new_pin > MAX_PIN_CODE) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  if (old_pin != g_root_device.pin_code) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_root_device.pin_code = new_pin;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetLockState(const RDMHeader *header,
                             UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = g_root_device.lock_state;
  // We don't include the unlocked state.
  *ptr++ = NUMBER_OF_LOCK_STATES - 1u;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetLockState(const RDMHeader *header,
                             const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint16_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t pin = ExtractUInt16(param_data);
  const uint8_t lock_state = param_data[2];
  if (pin != g_root_device.pin_code || lock_state >= NUMBER_OF_LOCK_STATES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_root_device.lock_state = lock_state;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetLockStateDescription(const RDMHeader *header,
                                        const uint8_t *param_data) {
  const uint8_t lock_state = param_data[0];
  if (lock_state == 0u || lock_state >= NUMBER_OF_LOCK_STATES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = lock_state;
  ptr += RDMUtil_StringCopy((char*) ptr, RDM_DEFAULT_STRING_SIZE,
                            LOCK_STATES[lock_state],
                            RDM_DEFAULT_STRING_SIZE);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_GetPresetInfo(const RDMHeader *header,
                              UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = true;  // level supported
  *ptr++ = true;  // sequence supported
  *ptr++ = true;  // split times supported
  *ptr++ = true;  // fail infinite delay supported
  *ptr++ = true;  // fail infinite hold supported
  *ptr++ = true;  // startup infinite hold supported
  ptr = PushUInt16(ptr, NUMBER_OF_SCENES);
  ptr = PushUInt16(ptr, 0u);  // min fade time
  ptr = PushUInt16(ptr, 0xfffe);  // max fade time
  ptr = PushUInt16(ptr, 0u);  // min wait time
  ptr = PushUInt16(ptr, 0xfffe);  // max wait time
  ptr = PushUInt16(ptr, 0u);  // min fail delay time
  ptr = PushUInt16(ptr, 0xfffe);  // max fail delay time
  ptr = PushUInt16(ptr, 0u);  // min fail hold time
  ptr = PushUInt16(ptr, 0xfffe);  // max fail hold time
  ptr = PushUInt16(ptr, 0u);  // min startup delay time
  ptr = PushUInt16(ptr, 0xfffe);  // max startup delay time
  ptr = PushUInt16(ptr, 0u);  // min startup hold time
  ptr = PushUInt16(ptr, 0xfffe);  // max startup hold time
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_GetPresetStatus(const RDMHeader *header,
                                UNUSED const uint8_t *param_data) {
  const uint16_t scene_index = ExtractUInt16(&param_data[0]);

  if (scene_index == 0u || scene_index > NUMBER_OF_SCENES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  Scene *scene = &g_root_device.scenes[scene_index - 1u];

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt16(ptr, scene_index);
  ptr = PushUInt16(ptr, scene->up_fade_time);
  ptr = PushUInt16(ptr, scene->down_fade_time);
  ptr = PushUInt16(ptr, scene->wait_time);
  *ptr++ = scene->programmed_state;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetPresetStatus(const RDMHeader *header,
                                const uint8_t *param_data) {
  if (header->param_data_length != 4u * sizeof(uint16_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t scene_index = ExtractUInt16(&param_data[0]);
  const uint16_t up_fade_time = ExtractUInt16(&param_data[2]);
  const uint16_t down_fade_time = ExtractUInt16(&param_data[4]);
  const uint16_t wait_time = ExtractUInt16(&param_data[6]);
  const uint8_t clear_preset = param_data[8];

  if (scene_index == 0u || scene_index > NUMBER_OF_SCENES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  Scene *scene = &g_root_device.scenes[scene_index - 1u];
  if (scene->programmed_state == PRESET_PROGRAMMED_READ_ONLY) {
    return RDMResponder_BuildNack(header, NR_WRITE_PROTECT);
  }

  if (clear_preset > 1u) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  if (clear_preset == 1u) {
    scene->up_fade_time = 0u;
    scene->down_fade_time = 0u;
    scene->wait_time = 0u;
    scene->programmed_state = PRESET_NOT_PROGRAMMED;
  } else {
    // don't change the state here, if we haven't been programmed, just update
    // the timing params
    scene->up_fade_time = up_fade_time;
    scene->down_fade_time = down_fade_time;
    scene->wait_time = wait_time;
  }
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetPresetMergeMode(const RDMHeader *header,
                                   UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetUInt8(header, g_root_device.merge_mode);
}

int DimmerModel_SetPresetMergeMode(const RDMHeader *header,
                                   const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t merge_mode = param_data[0];
  if (merge_mode > MERGE_MODE_DMX_ONLY) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_root_device.merge_mode = merge_mode;
  return RDMResponder_BuildSetAck(header);
}

// SubDevice PID Handlers
// ----------------------------------------------------------------------------
int DimmerModel_ClearStatusId(const RDMHeader *header,
                              UNUSED const uint8_t *param_data) {
  g_active_device->status_message.is_active = false;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetSubDeviceReportingThreshold(
    const RDMHeader *header,
    UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetUInt8(
      header, g_active_device->sd_report_threshold);
}

int DimmerModel_SetSubDeviceReportingThreshold(const RDMHeader *header,
                                               const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  uint8_t threshold = param_data[0];
  if (threshold != STATUS_NONE && threshold != STATUS_ADVISORY &&
      threshold != STATUS_WARNING && threshold != STATUS_ERROR) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_active_device->sd_report_threshold = threshold;
  return RDMResponder_BuildSetAck(header);
}

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

int DimmerModel_GetBurnIn(const RDMHeader *header,
                          UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetUInt8(header, g_active_device->burn_in);
}

int DimmerModel_SetBurnIn(const RDMHeader *header,
                          const uint8_t *param_data) {
  // TODO(simon): it would be nice to decrement this once an hour.
  return RDMResponder_GenericSetUInt8(header, param_data,
                                      &g_active_device->burn_in);
}

int DimmerModel_GetDimmerInfo(const RDMHeader *header,
                              UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt16(ptr, 0u);  // min level lower
  ptr = PushUInt16(ptr, 0xfffe);  // min level upper
  ptr = PushUInt16(ptr, 0u);  // max level lower
  ptr = PushUInt16(ptr, 0xfffe);  // max level upper
  *ptr++ = NUMBER_OF_CURVES;
  *ptr++ = 8u;  // level resolution
  *ptr++ = 1u;  // split levels supported
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_GetMinimumLevel(const RDMHeader *header,
                                UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt16(ptr, g_active_device->min_level_increasing);
  ptr = PushUInt16(ptr, g_active_device->min_level_decreasing);
  *ptr++ = g_active_device->on_below_min;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetMinimumLevel(const RDMHeader *header,
                                const uint8_t *param_data) {
  if (header->param_data_length != 2u * sizeof(uint16_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint16_t min_level_increasing = ExtractUInt16(&param_data[0]);
  const uint16_t min_level_decreasing = ExtractUInt16(&param_data[2]);
  const uint8_t on_below_min = param_data[4];

  if (on_below_min > 1) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_active_device->min_level_increasing = min_level_increasing;
  g_active_device->min_level_decreasing = min_level_decreasing;
  g_active_device->on_below_min = on_below_min;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetMaximumLevel(const RDMHeader *header,
                                UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericGetUInt16(header, g_active_device->max_level);
}

int DimmerModel_SetMaximumLevel(const RDMHeader *header,
                                const uint8_t *param_data) {
  return RDMResponder_GenericSetUInt16(header, param_data,
                                       &g_active_device->max_level);
}

int DimmerModel_GetCurve(const RDMHeader *header,
                         UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = g_active_device->curve;
  *ptr++ = NUMBER_OF_CURVES;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetCurve(const RDMHeader *header,
                         const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint8_t curve = param_data[0];
  if (curve == 0u || curve > NUMBER_OF_CURVES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  // To make it interesting, not every sub-device supports each curve type.
  if (curve % 2 && g_active_device->index % 2 == 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_active_device->curve = curve;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetCurveDescription(const RDMHeader *header,
                                    UNUSED const uint8_t *param_data) {
  const uint8_t curve = param_data[0];
  if (curve == 0u || curve > NUMBER_OF_CURVES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = curve;
  ptr += RDMUtil_StringCopy((char*) ptr, RDM_DEFAULT_STRING_SIZE,
                            DIMMER_CURVES[curve - 1],
                            RDM_DEFAULT_STRING_SIZE);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_GetOutputResponseTime(const RDMHeader *header,
                                      UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = g_active_device->output_response_time;
  *ptr++ = NUMBER_OF_OUTPUT_RESPONSE_TIMES;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetOutputResponseTime(const RDMHeader *header,
                                      const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint8_t setting = param_data[0];
  if (setting == 0u || setting > NUMBER_OF_OUTPUT_RESPONSE_TIMES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_active_device->output_response_time = setting;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetOutputResponseDescription(const RDMHeader *header,
                                             UNUSED const uint8_t *param_data) {
  const uint8_t setting = param_data[0];
  if (setting == 0u || setting > NUMBER_OF_OUTPUT_RESPONSE_TIMES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = setting;
  ptr += RDMUtil_StringCopy((char*) ptr, RDM_DEFAULT_STRING_SIZE,
                            OUTPUT_RESPONSE_TIMES[setting - 1],
                            RDM_DEFAULT_STRING_SIZE);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_GetModulationFrequency(const RDMHeader *header,
                                       UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = g_active_device->modulation_frequency;
  *ptr++ = NUMBER_OF_MODULATION_FREQUENCIES;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int DimmerModel_SetModulationFrequency(const RDMHeader *header,
                                       const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint8_t setting = param_data[0];
  if (setting == 0u || setting > NUMBER_OF_MODULATION_FREQUENCIES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_active_device->modulation_frequency = setting;
  return RDMResponder_BuildSetAck(header);
}

int DimmerModel_GetModulationFrequencyDescription(
    const RDMHeader *header,
    UNUSED const uint8_t *param_data) {
  const uint8_t setting = param_data[0];
  if (setting == 0u || setting > NUMBER_OF_MODULATION_FREQUENCIES) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  const ModulationFrequency *frequency = &MODULATION_FREQUENCY[setting - 1];
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = setting;
  ptr = PushUInt32(ptr, frequency->frequency);
  ptr += RDMUtil_StringCopy((char*) ptr, RDM_DEFAULT_STRING_SIZE,
                            frequency->description,
                            RDM_DEFAULT_STRING_SIZE);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

// Public Functions
// ----------------------------------------------------------------------------
void DimmerModel_Initialize() {
  // Initialize the root
  unsigned int i = 0u;
  for (; i != NUMBER_OF_SCENES; i++) {
    g_root_device.scenes[i].up_fade_time = 0u;
    g_root_device.scenes[i].down_fade_time = 0u;
    g_root_device.scenes[i].wait_time = 0u;
    g_root_device.scenes[i].programmed_state = i == 0u ?
        PRESET_PROGRAMMED_READ_ONLY : PRESET_NOT_PROGRAMMED;
  }

  g_root_device.playback_mode = PRESET_PLAYBACK_OFF;
  g_root_device.playback_level = 0u;
  g_root_device.startup_scene = PRESET_PLAYBACK_OFF;
  g_root_device.startup_hold = 0u;
  g_root_device.startup_delay = 0u;
  g_root_device.startup_level = 0u;
  g_root_device.fail_scene = PRESET_PLAYBACK_OFF;
  g_root_device.fail_loss_of_signal_delay = 0u;
  g_root_device.fail_hold_time = 0u;
  g_root_device.fail_level = 0u;
  g_root_device.pin_code = 0u;
  g_root_device.lock_state = 0u;
  g_root_device.merge_mode = MERGE_MODE_DEFAULT;
  g_root_device.power_on_self_test = false;
  g_root_device.running_self_test = SELF_TEST_OFF;

  // Initialize the subdevices.
  uint8_t parent_uid[UID_LENGTH];
  RDMResponder_GetUID(parent_uid);

  uint16_t sub_device_index = 1u;
  for (i = 0u; i < NUMBER_OF_SUB_DEVICES; i++) {
    if (i == 1) {
      // Leave a gap at sub-device 2, since sub devices aren't required to be
      // contiguous.
      sub_device_index++;
    }

    DimmerSubDevice *subdevice = &g_subdevices[i];
    subdevice->responder.def = &SUBDEVICE_RESPONDER_DEFINITION;

    subdevice->index = sub_device_index++;
    subdevice->min_level_increasing = 0u;
    subdevice->min_level_decreasing = 0u;
    subdevice->max_level = 0u;
    subdevice->on_below_min = 0u;
    subdevice->identify_mode = IDENTIFY_MODE_QUIET;
    subdevice->burn_in = 0u;
    subdevice->curve = 1u;
    subdevice->output_response_time = 1u;
    subdevice->modulation_frequency = 1u;
    subdevice->sd_report_threshold = STATUS_ADVISORY;
    subdevice->status_message.is_active = false;

    RDMResponder_SwitchResponder(&subdevice->responder);
    memcpy(g_responder->uid, parent_uid, UID_LENGTH);
    RDMResponder_InitResponder();
    g_responder->is_subdevice = true;
    g_responder->sub_device_count = NUMBER_OF_SUB_DEVICES;
  }

  // restore
  RDMResponder_RestoreResponder();
  if (!ResetToBlockAddress(INITIAL_START_ADDRESSS)) {
    // Set them all to 1
    for (i = 0u; i < NUMBER_OF_SUB_DEVICES; i++) {
      RDMResponder *responder = &g_subdevices[i].responder;
      responder->dmx_start_address = INITIAL_START_ADDRESSS;
    }
  }

  // init status messages
  g_status_messages.count = 0u;
}

static void DimmerModel_Activate() {
  g_responder->def = &ROOT_RESPONDER_DEFINITION;
  RDMResponder_InitResponder();
  g_responder->sub_device_count = NUMBER_OF_SUB_DEVICES;
  g_root_device.status_message_timer = CoarseTimer_GetTime();
}

static void DimmerModel_Deactivate() {}

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

  // Check if we're locked
  bool locked = false;
  if (header->command_class == SET_COMMAND) {
    if (g_root_device.lock_state == LOCK_STATE_ALL_LOCKED ||
        (g_root_device.lock_state == LOCK_STATE_SUBDEVICES_LOCKED &&
         sub_device != SUBDEVICE_ROOT)) {
      locked = true;
    }
  }

  if (sub_device == SUBDEVICE_ROOT) {
    if (locked) {
      return RDMResponder_BuildNack(header, NR_WRITE_PROTECT);
    } else {
      return RDMResponder_DispatchPID(header, param_data);
    }
  }

  unsigned int i = 0u;
  bool handled = false;
  int response_size = RDM_RESPONDER_NO_RESPONSE;
  for (; i < NUMBER_OF_SUB_DEVICES; i++) {
    if (sub_device == g_subdevices[i].index || sub_device == SUBDEVICE_ALL) {
      if (!locked) {
        g_active_device = &g_subdevices[i];
        RDMResponder_SwitchResponder(&g_subdevices[i].responder);
        response_size = RDMResponder_DispatchPID(header, param_data);
      }
      handled = true;
    }
  }

  RDMResponder_RestoreResponder();

  if (!handled) {
    return RDMResponder_BuildNack(header, NR_SUB_DEVICE_OUT_OF_RANGE);
  }

  if (locked) {
    return RDMResponder_BuildNack(header, NR_WRITE_PROTECT);
  }

  // If it was an all-subdevices call, it's not really clear how to handle the
  // response, in this case we return the last one.
  return response_size;
}

/*
 * @brief We generate status messages for each device, based on a periodic
 * timer. This makes it easier to reproduce problems (and test!).
 */
static void DimmerModel_Tasks() {
  // The cycle counter is used to generate status messages for each sub device.
  static uint8_t cycle = 0u;
  static uint16_t complete_cycles = 0u;

  if (g_root_device.running_self_test &&
      CoarseTimer_HasElapsed(
          g_root_device.self_test_timer,
          SELF_TESTS[g_root_device.running_self_test].duration)) {
    // Queue a status message for the root.
    QueueStatusMessage(
        &g_root_device.status_message, SUBDEVICE_ROOT, STATUS_ADVISORY,
        (uint16_t) (g_root_device.running_self_test == 1u ?
            STS_OLP_SELFTEST_PASSED : STS_OLP_SELFTEST_FAILED),
        g_root_device.running_self_test, 0u);

    g_root_device.running_self_test = 0u;
  }

  if (!CoarseTimer_HasElapsed(g_root_device.status_message_timer,
                             STATUS_MESSAGE_TRIGGER_INTERVAL)) {
    return;
  }

  g_root_device.status_message_timer = CoarseTimer_GetTime();

  unsigned int subdevice_index = 0u;
  for (; subdevice_index < NUMBER_OF_SUB_DEVICES; subdevice_index++) {
    DimmerSubDevice *subdevice = &g_subdevices[subdevice_index];

    if (subdevice->index == 1u) {
      // The cycle for the first device is:
      //  - 0, NOOP
      //  - 1, Queue breaker trip warning
      //  - 2, NOOP
      //  - 3, Clear breaker trip warning
      //  - 4, NOOP
      if (cycle == 1) {
        // Queue a message
        QueueSubDeviceStatusMessage(subdevice, STATUS_WARNING,
                                    STS_BREAKER_TRIP, 0u, 0u);
      } else if (cycle == 3u) {
        if (subdevice->status_message.is_active) {
          // The previous message is still in the queue, cancel it.
          subdevice->status_message.is_active = false;
        } else {
          // Queue a 'cleared' message
          QueueSubDeviceStatusMessage(subdevice, STATUS_WARNING_CLEARED,
                                      STS_BREAKER_TRIP, 0u, 0u);
        }
      }
    } else if (subdevice->index == 3u) {
      // This subdevice just queues a manufacturer-defined advisory message
      // each cycle.
      QueueSubDeviceStatusMessage(subdevice, STATUS_ADVISORY,
                                  (uint16_t) STS_OLP_TESTING,
                                  complete_cycles, cycle);
    }
  }
  cycle++;
  cycle %= 5u;
  if (cycle == 0u) {
    complete_cycles++;
  }
}

const ModelEntry DIMMER_MODEL_ENTRY = {
  .model_id = DIMMER_MODEL_ID,
  .activate_fn = DimmerModel_Activate,
  .deactivate_fn = DimmerModel_Deactivate,
  .ioctl_fn = RDMResponder_Ioctl,
  .request_fn = DimmerModel_HandleRequest,
  .tasks_fn = DimmerModel_Tasks
};

// Root device definition
// ----------------------------------------------------------------------------

static const PIDDescriptor ROOT_PID_DESCRIPTORS[] = {
  {PID_STATUS_MESSAGES, DimmerModel_GetStatusMessages, 1u,
    (PIDCommandHandler) NULL},
  {PID_STATUS_ID_DESCRIPTION, DimmerModel_GetStatusIdDescription, 2u,
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
  {PID_PERFORM_SELFTEST, DimmerModel_GetSelfTest, 0u,
    DimmerModel_PerformSelfTest},
  {PID_SELF_TEST_DESCRIPTION, DimmerModel_GetSelfTestDescription, 1u,
    (PIDCommandHandler) NULL},
  {PID_CAPTURE_PRESET, (PIDCommandHandler) NULL, 0,
    DimmerModel_CapturePreset},
  {PID_PRESET_PLAYBACK, DimmerModel_GetPresetPlayback, 0,
    DimmerModel_SetPresetPlayback},
  {PID_DMX_BLOCK_ADDRESS, DimmerModel_GetDMXBlockAddress, 0u,
    DimmerModel_SetDMXBlockAddress},
  {PID_DMX_FAIL_MODE, DimmerModel_GetDMXFailMode, 0u,
    DimmerModel_SetDMXFailMode},
  {PID_DMX_STARTUP_MODE, DimmerModel_GetDMXStartupMode, 0u,
    DimmerModel_SetDMXStartupMode},
  {PID_LOCK_PIN, DimmerModel_GetLockPin, 0u, DimmerModel_SetLockPin},
  {PID_LOCK_STATE, DimmerModel_GetLockState, 0u, DimmerModel_SetLockState},
  {PID_LOCK_STATE_DESCRIPTION, DimmerModel_GetLockStateDescription, 1u,
    (PIDCommandHandler) NULL},
  {PID_PRESET_INFO, DimmerModel_GetPresetInfo, 0u,
    (PIDCommandHandler) NULL},
  {PID_PRESET_STATUS, DimmerModel_GetPresetStatus, 2u,
    DimmerModel_SetPresetStatus},
  {PID_PRESET_MERGEMODE, DimmerModel_GetPresetMergeMode, 0u,
    DimmerModel_SetPresetMergeMode},
  {PID_POWER_ON_SELF_TEST, DimmerModel_GetPowerOnSelfTest, 0u,
    DimmerModel_SetPowerOnSelfTest}
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
  .model_id = DIMMER_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT
};


// Sub device definitions
// ----------------------------------------------------------------------------

static const PIDDescriptor SUBDEVICE_PID_DESCRIPTORS[] = {
  {PID_CLEAR_STATUS_ID, (PIDCommandHandler) NULL, 0u,
    DimmerModel_ClearStatusId},
  {PID_SUB_DEVICE_STATUS_REPORT_THRESHOLD,
    DimmerModel_GetSubDeviceReportingThreshold, 1u,
    DimmerModel_SetSubDeviceReportingThreshold},
  {PID_SUPPORTED_PARAMETERS, RDMResponder_GetSupportedParameters, 0u,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_INFO, RDMResponder_GetDeviceInfo, 0u, (PIDCommandHandler) NULL},
  {PID_PRODUCT_DETAIL_ID_LIST, RDMResponder_GetProductDetailIds, 0u,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_MODEL_DESCRIPTION, RDMResponder_GetDeviceModelDescription, 0u,
    (PIDCommandHandler) NULL},
  {PID_MANUFACTURER_LABEL, RDMResponder_GetManufacturerLabel, 0u,
    (PIDCommandHandler) NULL},
  {PID_DMX_START_ADDRESS, RDMResponder_GetDMXStartAddress, 0u,
    RDMResponder_SetDMXStartAddress},
  {PID_SOFTWARE_VERSION_LABEL, RDMResponder_GetSoftwareVersionLabel, 0u,
    (PIDCommandHandler) NULL},
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0u,
    RDMResponder_SetIdentifyDevice},
  {PID_BURN_IN, DimmerModel_GetBurnIn, 0u, DimmerModel_SetBurnIn},
  {PID_IDENTIFY_MODE, DimmerModel_GetIdentifyMode, 0u,
    DimmerModel_SetIdentifyMode},
  {PID_DIMMER_INFO, DimmerModel_GetDimmerInfo, 0u,
    (PIDCommandHandler) NULL},
  {PID_MINIMUM_LEVEL, DimmerModel_GetMinimumLevel, 0u,
    DimmerModel_SetMinimumLevel},
  {PID_MAXIMUM_LEVEL, DimmerModel_GetMaximumLevel, 0u,
    DimmerModel_SetMaximumLevel},
  {PID_CURVE, DimmerModel_GetCurve, 0u, DimmerModel_SetCurve},
  {PID_CURVE_DESCRIPTION, DimmerModel_GetCurveDescription, 1u,
    (PIDCommandHandler) NULL},
  {PID_OUTPUT_RESPONSE_TIME, DimmerModel_GetOutputResponseTime, 0u,
    DimmerModel_SetOutputResponseTime},
  {PID_OUTPUT_RESPONSE_TIME_DESCRIPTION,
    DimmerModel_GetOutputResponseDescription, 1u, (PIDCommandHandler) NULL},
  {PID_MODULATION_FREQUENCY, DimmerModel_GetModulationFrequency, 0u,
    DimmerModel_SetModulationFrequency},
  {PID_MODULATION_FREQUENCY_DESCRIPTION,
    DimmerModel_GetModulationFrequencyDescription, 1u,
    (PIDCommandHandler) NULL},
};

static const ProductDetailIds SUBDEVICE_PRODUCT_DETAIL_ID_LIST = {
  .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL},
  .size = 2u
};

static const char SLOT_DIMMER_DESCRIPTION[] = "Dimmer";

static const SlotDefinition PERSONALITY_SLOTS[] = {
  {
    .description = SLOT_DIMMER_DESCRIPTION,
    .slot_label_id = SD_INTENSITY,
    .slot_type = ST_PRIMARY,
    .default_value = 0u,
  }
};

static const PersonalityDefinition PERSONALITIES[PERSONALITY_COUNT] = {
  {
    .dmx_footprint = 1u,
    .description = PERSONALITY_DESCRIPTION,
    .slots = PERSONALITY_SLOTS,
    .slot_count = 1u
  },
};

static const ResponderDefinition SUBDEVICE_RESPONDER_DEFINITION = {
  .descriptors = SUBDEVICE_PID_DESCRIPTORS,
  .descriptor_count = sizeof(SUBDEVICE_PID_DESCRIPTORS) / sizeof(PIDDescriptor),
  .sensors = NULL,
  .sensor_count = 0u,
  .personalities = PERSONALITIES,
  .personality_count = 1u,
  .software_version_label = SOFTWARE_LABEL,
  .manufacturer_label = MANUFACTURER_LABEL,
  .model_description = DEVICE_MODEL_DESCRIPTION,
  .product_detail_ids = &SUBDEVICE_PRODUCT_DETAIL_ID_LIST,
  .default_device_label = DEFAULT_DEVICE_LABEL,
  .software_version = SOFTWARE_VERSION,
  .model_id = DIMMER_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT
};
