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
 * rdm_responder.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup rdm
 * @{
 * @file rdm_responder.h
 * @brief The base RDM Responder.
 *
 * The base RDM Responder provides the common RDM handling code used by many of
 * the responder models. This allows much of the responder's functionality to be
 * specified with a declarative style, rather that reimplementing the PID
 * handlers many times over.
 *
 * It consists of a couple of parts:
 *  - A ResponderDefinition, a static tree of data structures that describe
 *    how the responder should behave. This contains the responder's
 *    manufacturer id, model id, sensors definitions, personalities definitions,
 *    supported parameters list etc.
 *  - the global RDMResponder g_responder object, which holds basic state like
 *    mute, identify etc.
 *  - The PID dispatching mechanism, where we specifiy a table of function
 *    pointers as part of the responder definition and then later, when a RDM
 *    request arrives, RDMResponder_DispatchPID() is called which invokes the
 *    correct function. You can think of this like a vtable in C++.
 *
 * When implementing a model, you can reference the PID functions in the
 * dispatch table, or point to your own functions that (optionally) wrap the
 * PID functions.
 */

#ifndef FIRMWARE_SRC_RDM_RESPONDER_H_
#define FIRMWARE_SRC_RDM_RESPONDER_H_

#include <stdbool.h>
#include <stdint.h>

#include "system_config.h"

#include "peripheral/ports/plib_ports.h"
#include "rdm.h"
#include "rdm_frame.h"
#include "rdm_handler.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The common MANUFACTURER_LABEL.
 */
extern const char MANUFACTURER_LABEL[];

/**
 * @brief A PID handler.
 * @param incoming_header The header for the request.
 * @param param_data The parameter data, or NULL if there isn't any.
 * @returns The size of the RDM response in g_rdm_buffer. Use
 * RDM_RESPONDER_NO_RESPONSE if there is no response.
 */
typedef int (*PIDCommandHandler)(const RDMHeader *incoming_header,
                                 const uint8_t *param_data);

/**
 * @brief A descriptor for a PID.
 *
 * This contains the value of the parameter, and a GET / SET function pointer.
 */
typedef struct {
  /**
   * @brief The parameter ID.
   */
  uint16_t pid;

  /**
   * @brief The handler to use for GET requests.
   *
   * If NULL, NR_UNSUPPORTED_COMMAND_CLASS will be returned instead.
   */
  PIDCommandHandler get_handler;

  /**
   * @brief The size of the parameter data for GET requests.
   */
  uint8_t get_param_size;

  /**
   * @brief The handler to use for SET requests.
   *
   * If NULL, NR_UNSUPPORTED_COMMAND_CLASS will be returned instead.
   */
  PIDCommandHandler set_handler;
} PIDDescriptor;

/**
 * @brief The Product Detail IDs for the responder.
 *
 * This is used in PRODUCT_DETAIL_ID_LIST.
 */
typedef struct {
  /**
   * @brief An array of Product Detail IDs.
   */
  RDMProductDetail ids[MAX_PRODUCT_DETAILS];
  uint8_t size;  //!< The number of ids in the array.
} ProductDetailIds;

/**
 * @brief The definition of a DMX slot.
 */
typedef struct {
  /**
   * @brief The string description of the slot.
   */
  const char *description;

  /**
   * @brief The slot label id.
   */
  uint16_t slot_label_id;

  /**
   * @brief The slot type.
   */
  uint8_t slot_type;

  /**
   * @brief The default slot value.
   */
  uint8_t default_value;
} SlotDefinition;

/**
 * @brief The definition of a DMX512 personality.
 */
typedef struct {
  /**
   * @brief The number of slots this personality requires.
   */
  uint16_t dmx_footprint;

  /**
   * @brief The string description of the personality.
   */
  const char *description;

  /**
   * @brief The slot definitions.
   *
   * This may be NULL if slot definitions aren't provided.
   */
  const SlotDefinition *slots;

  /**
   * @brief The number of slot definitions.
   */
  uint16_t slot_count;
} PersonalityDefinition;

/**
 * @brief An RDM sensor definition.
 *
 * This contains all the information found in SENSOR_DEFINITION.
 */
typedef struct {
  const char *description;  //!< Pointer to the sensor description
  int16_t range_maximum_value;  //!< The max value of the sensor
  int16_t range_minimum_value;  //!< The min value of the sensor
  int16_t normal_maximum_value;  //!< The max normal range of the sensor
  int16_t normal_minimum_value;  //!< The min normal range of the sensor
  uint8_t recorded_value_support;  //!< Recorded support bitfield (see E1.20)
  RDMSensorType type;  //!< The sensor type
  RDMUnit unit;  //!< The units for the sensor values.
  RDMPrefix prefix;  //!< The prefix for the sensor values.
} SensorDefinition;

/**
 * @brief Data for an RDM Sensor.
 */
typedef struct {
  int16_t present_value;  //!< The current value of the sensor
  int16_t lowest_value;  //!< The lowest recorded value.
  int16_t highest_value;  //!< The highest recorded value.
  int16_t recorded_value;  //!< The saved 'snapshot' value.
  /**
   * @brief Optional NACK reason, used if should_nack is true.
   */
  RDMNackReason nack_reason;

  bool should_nack;  //!< True if we should NACK SENSOR_VALUE requests.
} SensorData;

/**
 * @brief The definition of a responder.
 *
 * This contains the PID dispatch table, and const data, like the
 * manufacturer name, device model etc, sensor definitions, etc.
 */
typedef struct {
  /**
   * @brief The descriptor table.
   */
  const PIDDescriptor *descriptors;

  /**
   * @brief The sensor definitions table.
   *
   * This may be NULL if the responder does not have sensors.
   */
  const SensorDefinition *sensors;

  /**
   * @brief The personality definition table.
   *
   * This may be NULL if the responder does not have personalities.
   */
  const PersonalityDefinition *personalities;

  const char *software_version_label;  //!< The software version label.
  const char *manufacturer_label;  //!< The manufacturer label.
  const char *model_description;  //!< The model description.
  const char *default_device_label;  //!< The defaut device label.

  /**
   * @brief The list of product IDs.
   */
  const ProductDetailIds *product_detail_ids;

  /**
   * @brief The number of descriptors in the table.
   */
  unsigned int descriptor_count;

  /**
   * @brief The number of personality definitions in the table.
   */
  unsigned int personality_count;

  uint32_t software_version;  //!< The Software version.
  uint16_t model_id;  //!< The model ID.
  RDMProductCategory product_category;  //!< The product category.

  /**
   * @brief The number of sensor definitions in the table.
   */
  uint8_t sensor_count;  //!< The number of sensors
} ResponderDefinition;

/**
 * @brief A core implementation of a responder.
 *
 * This contains the mutable state for a responder.
 */
typedef struct {
  char device_label[RDM_DEFAULT_STRING_SIZE];  //!< Device label
  uint8_t uid[UID_LENGTH];  //!< Responder's UID

  /**
   * @brief The ResponderDefinition
   */
  const ResponderDefinition *def;

  /**
   * @brief A pointer to an array of SensorData structs.
   *
   * The array must be the same size as the SensorDefinitions in
   * ResponderDefinition.
   */
  SensorData *sensors;

  uint16_t dmx_start_address;  //!< DMX start address
  uint16_t sub_device_count;  //!< The number of sub devices
  uint8_t current_personality;  //!< Current DMX personality, 1-indexed.
  uint8_t queued_message_count;  //!< queued message count.
  bool is_muted;  //!< The mute state for the responder
  bool identify_on;  //!< The identify state for the responder.
  bool using_factory_defaults;  //!< True if using factory defaults.
  bool is_subdevice;  // true if this is a subdevice.
  bool is_managed_proxy;  // true if this is a managed proxy.
  bool is_proxied_device;  // true if this is a proxied device.
} RDMResponder;

/**
 * @brief The global RDMResponder object.
 */
extern RDMResponder *g_responder;

/**
 * @brief Indicates there is no response required for the request.
 */
static const int RDM_RESPONDER_NO_RESPONSE = 0;

/**
 * @brief The settings to use for the RDM Responder.
 */
typedef struct {
  PORTS_CHANNEL identify_port;  //!< The port to use for the identify signal.
  PORTS_BIT_POS identify_bit;  //!< The port bit to use for the identify signal.
  PORTS_CHANNEL mute_port;  //!< The port to use to indicate mute state.
  PORTS_BIT_POS mute_bit;  //!< The port bit used to indicate mute state.
  uint8_t uid[UID_LENGTH];  //!< The responder's UID.
} RDMResponderSettings;

/**
 * @brief Initialize an RDMResponder struct.
 * @param settings the settings to use for the responder.
 */
void RDMResponder_Initialize(const RDMResponderSettings *settings);

/**
 * @brief Perform the periodic tasks.
 *
 * This should be called in the main event loop.
 */
void RDMResponder_Tasks();

/**
 * @brief Reset an RDMResponder to the factory defaults.
 */
void RDMResponder_ResetToFactoryDefaults();

/**
 * @brief Get the UID of the responder.
 * @param uid A pointer to copy the UID to; should be at least UID_LENGTH.
 */
void RDMResponder_GetUID(uint8_t *uid);

/**
 * @brief Handle a Discovery-unique-branch request.
 * @param param_data The DUB request param_data.
 * @param param_data_length The size of the param_data.
 * @returns The size of the RDM response frame, this will be negative to
 *   indicate no break should be sent.
 */
int RDMResponder_HandleDUBRequest(const uint8_t *param_data,
                                  unsigned int param_data_length);

/**
 * @brief Build the RDM header in the output buffer.
 * @param incoming_header The header of the incoming frame.
 * @param response_type The response type to use.
 * @param command_class The command class to use.
 * @param pid the PID to use.
 * @param param_data_length The length of the parameter data.
 */
void RDMResponder_BuildHeader(const RDMHeader *incoming_header,
                              RDMResponseType response_type,
                              RDMCommandClass command_class,
                              uint16_t pid,
                              unsigned int param_data_length);

int RDMResponder_AddHeaderAndChecksum(const RDMHeader *incoming_header,
                                      RDMResponseType response_type,
                                      unsigned int message_length);

/**
 * @brief Handle discovery commands.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame. A negative value means no break
 *   should be sent.
 */
int RDMResponder_HandleDiscovery(const RDMHeader *incoming_header,
                                 const uint8_t *param_data);

/**
 * @brief Build an RDM Set ACK with no param data.
 * @param incoming_header The header of the incoming frame.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_BuildSetAck(const RDMHeader *incoming_header);

/**
 * @brief Build an RDM NACK.
 * @param incoming_header The header of the incoming frame.
 * @param reason The NACK reason code.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_BuildNack(const RDMHeader *incoming_header,
                           RDMNackReason reason);

/**
 * @brief Build an ACK TIMER
 * @param incoming_header The header of the incoming frame.
 * @param delay The ack timer delay in 10ths of a second.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_BuildAckTimer(const RDMHeader *incoming_header,
                               uint16_t delay);

/**
 * @brief Invoke a PID handler from the ResponderDefinition.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 *
 * This checks the ResponderDefinition for a matching PID handler of the
 * correct command class. If one isn't found, it'll NACK with
 * NR_UNSUPPORTED_COMMAND_CLASS or NR_UNKNOWN_PID.
 */
int RDMResponder_DispatchPID(const RDMHeader *incoming_header,
                             const uint8_t *param_data);

/**
 * @brief A base Ioctl handler.
 * @param command The ioctl command to run.
 * @param data arbitary data, depends on the ModelIoctl.
 * @param length the size of the data.
 * @returns An int, the meaning of which depends on the ModelIoctl.
 */
int RDMResponder_Ioctl(ModelIoctl command, uint8_t *data,
                       unsigned int length);

// PID Handlers
// ----------------------------------------------------------------------------

/**
 * @brief Build a response containing a string.
 * @param incoming_header The header of the incoming frame.
 * @param reply_string The string to reply with
 * @param max_size The maximum size of the string. We may return a shorter
 * string if it contains a NULL.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericReturnString(const RDMHeader *incoming_header,
                                     const char *reply_string,
                                     unsigned int max_size);

/**
 * @brief Handle a request to get a bool value.
 * @param incoming_header The header of the incoming frame.
 * @param value The bool value
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericGetBool(const RDMHeader *incoming_header,
                                bool value);

/**
 * @brief Handle a request to set a bool value.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @param value The bool value to set
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericSetBool(const RDMHeader *incoming_header,
                                const uint8_t *param_data,
                                bool *value);

/**
 * @brief Handle a request to get a uint8_t value.
 * @param incoming_header The header of the incoming frame.
 * @param value The uint8_t value to return.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericGetUInt8(const RDMHeader *incoming_header,
                                 uint8_t value);

/**
 * @brief Handle a request to set a uint8_t value.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @param value The uint8_t value to set.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericSetUInt8(const RDMHeader *incoming_header,
                                 const uint8_t *param_data,
                                 uint8_t *value);

/**
 * @brief Handle a request to get a uint16_t value.
 * @param incoming_header The header of the incoming frame.
 * @param value The uint16_t value to return.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericGetUInt16(const RDMHeader *incoming_header,
                                  uint16_t value);

/**
 * @brief Handle a request to set a uint16_t value.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @param value The uint16_t value to set.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericSetUInt16(const RDMHeader *incoming_header,
                                  const uint8_t *param_data,
                                  uint16_t *value);

/**
 * @brief Handle a request to get a uint32_t value.
 * @param incoming_header The header of the incoming frame.
 * @param value The unsigned int value
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericGetUInt32(const RDMHeader *incoming_header,
                                  uint32_t value);

/**
 * @brief Handle a request to set a uint32_t value.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @param value The uint32_t value to set
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericSetUInt32(const RDMHeader *incoming_header,
                                  const uint8_t *param_data,
                                  uint32_t *value);

/**
 * @brief Handle a SET MUTE request.
 * @param incoming_header The header of the incoming frame.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_SetMute(const RDMHeader *incoming_header);

/**
 * @brief Handle a SET UN_MUTE request.
 * @param incoming_header The header of the incoming frame.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_SetUnMute(const RDMHeader *incoming_header);

/**
 * @brief Handle a GET DEVICE_INFO request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetDeviceInfo(const RDMHeader *incoming_header,
                               const uint8_t *param_data);

/**
 * @brief Handle a SUPPORTED_PARAMETERS request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetSupportedParameters(const RDMHeader *incoming_header,
                                        const uint8_t *param_data);

/**
 * @brief Handle a GET PRODUCT_DETAIL_IDS request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetProductDetailIds(const RDMHeader *incoming_header,
                                     const uint8_t *param_data);

/**
 * @brief Handle a GET DEVICE_MODEL_DESCRIPTION request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetDeviceModelDescription(const RDMHeader *incoming_header,
                                           const uint8_t *param_data);

/**
 * @brief Handle a GET MANUFACTURER_LABEL request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetManufacturerLabel(const RDMHeader *incoming_header,
                                      const uint8_t *param_data);

/**
 * @brief Handle a GET SOFTWARE_VERSION_LABEL request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetSoftwareVersionLabel(const RDMHeader *incoming_header,
                                         const uint8_t *param_data);

/**
 * @brief Handle a GET DEVICE_LABEL request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetDeviceLabel(const RDMHeader *incoming_header,
                                const uint8_t *param_data);

/**
 * @brief
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_SetDeviceLabel(const RDMHeader *incoming_header,
                                const uint8_t *param_data);

/**
 * @brief Handle a GET DMX_PERSONALITY request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetDMXPersonality(const RDMHeader *incoming_header,
                                   const uint8_t *param_data);

/**
 * @brief Handle a SET DMX_PERSONALITY request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_SetDMXPersonality(const RDMHeader *incoming_header,
                                   const uint8_t *param_data);

/**
 * @brief Handle a GET DMX_PERSONALITY_DESCRIPTION request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetDMXPersonalityDescription(const RDMHeader *incoming_header,
                                              const uint8_t *param_data);

/**
 * @brief Handle a GET DMX_START_ADDRESS request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetDMXStartAddress(const RDMHeader *incoming_header,
                                    const uint8_t *param_data);

/**
 * @brief Handle a SET DMX_START_ADDRESS request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_SetDMXStartAddress(const RDMHeader *incoming_header,
                                    const uint8_t *param_data);

/**
 * @brief Handle a GET SLOT_INFO request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetSlotInfo(const RDMHeader *incoming_header,
                             const uint8_t *param_data);

/**
 * @brief Handle a GET SLOT_DESCRIPTION request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetSlotDescription(const RDMHeader *incoming_header,
                                    const uint8_t *param_data);

/**
 * @brief Handle a GET DEFAULT_SLOT_VALUE request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetDefaultSlotValue(const RDMHeader *incoming_header,
                                     const uint8_t *param_data);


/**
 * @brief Handle a GET SENSOR_DEFINITION request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetSensorDefinition(const RDMHeader *incoming_header,
                                     const uint8_t *param_data);

/**
 * @brief Handle a GET SENSOR_VALUE request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetSensorValue(const RDMHeader *incoming_header,
                                const uint8_t *param_data);

/**
 * @brief Handle a SET SENSOR_VALUE request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_SetSensorValue(const RDMHeader *incoming_header,
                                const uint8_t *param_data);

/**
 * @brief Handle a SET RECORD_SENSOR request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_SetRecordSensor(const RDMHeader *incoming_header,
                                 const uint8_t *param_data);

/**
 * @brief Handle a GET IDENTIFY_DEVICE request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GetIdentifyDevice(const RDMHeader *incoming_header,
                                   const uint8_t *param_data);

/**
 * @brief Handle a SET IDENTIFY_DEVICE request.
 * @param incoming_header The header of the incoming frame.
 * @param param_data The received parameter data.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_SetIdentifyDevice(const RDMHeader *incoming_header,
                                   const uint8_t *param_data);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_RDM_RESPONDER_H_
