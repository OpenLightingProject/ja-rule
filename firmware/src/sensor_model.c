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
 * sensor_model.c
 * Copyright (C) 2015 Simon Newton
 */
#include "sensor_model.h"

#include <stdlib.h>

#include "coarse_timer.h"
#include "constants.h"
#include "rdm_frame.h"
#include "rdm_responder.h"
#include "rdm_util.h"
#include "utils.h"

// Various constants

enum { NUMBER_OF_SENSORS = 3 };
enum { SOFTWARE_VERSION = 0x00000000u };

static const uint32_t SENSOR_SAMPLE_RATE = 10000u;
static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule Sensor Device";
static const char SOFTWARE_LABEL[] = "Alpha";
static const char DEFAULT_DEVICE_LABEL[] = "Ja Rule";
static const char SENSOR_NAME1[] = "Temperature";
static const char SENSOR_NAME2[] = "Missing Sensor";
static const char SENSOR_NAME3[] = "Voltage";

static const ResponderDefinition RESPONDER_DEFINITION;

/*
 * @brief The sensor model state.
 */
typedef struct {
  CoarseTimer_Value sensor_sample_time;
  SensorData sensors[NUMBER_OF_SENSORS];
} SensorModel;

static SensorModel g_sensor_model;

void SampleSensors() {
  g_sensor_model.sensor_sample_time = CoarseTimer_GetTime();

  unsigned int i = 0;
  for (; i < NUMBER_OF_SENSORS; i++) {
    int16_t new_value = (Random_PseudoGet() % (
        RESPONDER_DEFINITION.sensors[i].range_maximum_value -
        RESPONDER_DEFINITION.sensors[i].range_minimum_value)) +
        RESPONDER_DEFINITION.sensors[i].range_minimum_value;
    RDMUtil_UpdateSensor(
        &g_sensor_model.sensors[i],
        RESPONDER_DEFINITION.sensors[i].recorded_value_support,
        new_value);
  }
}

// Public Functions
// ----------------------------------------------------------------------------
void SensorModel_Initialize() {}

static void SensorModel_Activate() {
  g_responder.def = &RESPONDER_DEFINITION;
  unsigned int i = 0u;
  for (; i < NUMBER_OF_SENSORS; i++) {
    g_sensor_model.sensors[i].present_value = 0u;
    g_sensor_model.sensors[i].lowest_value = 0u;
    g_sensor_model.sensors[i].highest_value = 0u;
    g_sensor_model.sensors[i].recorded_value = 0u;
    if (i == 1) {
      // The 2nd sensor (index 1) always nacks with a HARDWARE_FAULT
      g_sensor_model.sensors[i].should_nack = true;
      g_sensor_model.sensors[i].nack_reason = NR_HARDWARE_FAULT;
    } else {
      g_sensor_model.sensors[i].should_nack = false;
    }
  }

  RDMResponder_ResetToFactoryDefaults();
  SampleSensors();
  g_responder.sensors = g_sensor_model.sensors;
}

static void SensorModel_Deactivate() {}

static int SensorModel_Ioctl(ModelIoctl command, uint8_t *data,
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

static int SensorModel_HandleRequest(const RDMHeader *header,
                                     const uint8_t *param_data) {
  if (!RDMUtil_RequiresAction(g_responder.uid, header->dest_uid)) {
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

static void SensorModel_Tasks() {
  if (CoarseTimer_HasElapsed(g_sensor_model.sensor_sample_time,
                             SENSOR_SAMPLE_RATE)) {
    SampleSensors();
  }
}

const ModelEntry SENSOR_MODEL_ENTRY = {
  .model_id = SENSOR_MODEL_ID,
  .activate_fn = SensorModel_Activate,
  .deactivate_fn = SensorModel_Deactivate,
  .ioctl_fn = SensorModel_Ioctl,
  .request_fn = SensorModel_HandleRequest,
  .tasks_fn = SensorModel_Tasks
};

static const PIDDescriptor PID_DESCRIPTORS[] = {
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
  {PID_SENSOR_DEFINITION, RDMResponder_GetSensorDefinition, 1u,
    (PIDCommandHandler) NULL},
  {PID_SENSOR_VALUE, RDMResponder_GetSensorValue, 1u,
    RDMResponder_SetSensorValue},
  {PID_RECORD_SENSORS, (PIDCommandHandler) NULL, 0u,
    RDMResponder_SetRecordSensor},
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0u,
    RDMResponder_SetIdentifyDevice}
};

static const ProductDetailIds PRODUCT_DETAIL_ID_LIST = {
  .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL},
  .size = 2
};

static const SensorDefinition SENSOR_DEFINITIONS[] = {
  {
    .description = SENSOR_NAME1,
    .normal_maximum_value = 50,
    .normal_minimum_value = 0,
    .range_maximum_value = 100,
    .range_minimum_value = -10,
    .recorded_value_support = SENSOR_SUPPORTS_RECORDING_MASK |
        SENSOR_SUPPORTS_LOWEST_HIGHEST_MASK,
    .type = SENSOR_TEMPERATURE,
    .unit = UNITS_CENTIGRADE,
    .prefix = PREFIX_NONE
  },
  {
    .description = SENSOR_NAME2,
    .normal_maximum_value = 196,  // +1G
    .normal_minimum_value = 0,  // -1G
    .range_maximum_value = 882,  // +8G
    .range_minimum_value = -686,  // -8G
    .recorded_value_support = 0u,
    .type = SENSOR_ACCELERATION,
    .unit = UNITS_METERS_PER_SECOND_SQUARED,
    .prefix = PREFIX_DECI
  },
  {
    .description = SENSOR_NAME3,
    .normal_maximum_value = 35,
    .normal_minimum_value = 30,
    .range_maximum_value = 50,
    .range_minimum_value = 0,
    .recorded_value_support = SENSOR_SUPPORTS_LOWEST_HIGHEST_MASK,
    .type = SENSOR_VOLTAGE,
    .unit = UNITS_VOLTS_DC,
    .prefix = PREFIX_MILLI
  },
};

static const ResponderDefinition RESPONDER_DEFINITION = {
  .descriptors = PID_DESCRIPTORS,
  .descriptor_count = sizeof(PID_DESCRIPTORS) / sizeof(PIDDescriptor),
  .sensors = SENSOR_DEFINITIONS,
  .sensor_count = NUMBER_OF_SENSORS,
  .personalities = NULL,
  .personality_count = 0u,
  .software_version_label = SOFTWARE_LABEL,
  .manufacturer_label = MANUFACTURER_LABEL,
  .model_description = DEVICE_MODEL_DESCRIPTION,
  .product_detail_ids = &PRODUCT_DETAIL_ID_LIST,
  .default_device_label = DEFAULT_DEVICE_LABEL,
  .software_version = SOFTWARE_VERSION,
  .model_id = SENSOR_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT
};
