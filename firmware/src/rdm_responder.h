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
 * The base RDM Responder provides the building blocks for implementing
 * responder models. You can think of it as a base class if we were using C++.
 *
 * It consists of a couple of parts:
 *  - the global g_responder object, which holds basic state like mute,
 *    identify etc.
 *  - The PID dispatching mechanism, where we specifiy a table of function
 *    pointers and then call RDMResponder_DispatchPID().
 *  - Functions for various common PIDs.
 *
 * When implementing a model, you can reference the PID functions in the
 * dispatch table, or point to your own functions that (optionally) wrap the
 * PID functions.
 */

#ifndef FIRMWARE_SRC_RDM_RESPONDER_H_
#define FIRMWARE_SRC_RDM_RESPONDER_H_

#include <stdbool.h>
#include <stdint.h>

#include "rdm.h"
#include "rdm_frame.h"

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
 * This contains the PID, and a GET / SET handler.
 */
typedef struct {
  uint16_t pid;  //!< The parameter ID.
  PIDCommandHandler get_handler;  //!< The GET handler.
  PIDCommandHandler set_handler;  //!< The SET handler.
} PIDDescriptor;

/**
 * @brief The Product Detail IDs for the responder.
 */
typedef struct {
  /**
   * @brief An array of Product Detail IDs.
   */
  RDMProductDetail ids[MAX_PRODUCT_DETAILS];
  uint8_t size;  //!< The number of ids in the array.
} ProductDetailIds;

/**
 * @brief The definition of a responder.
 *
 * This contains the PID dispatch table, and read-only variables, like the
 * manufacturer name, device model etc.
 */
typedef struct {
  const PIDDescriptor *descriptors;
  const unsigned int descriptor_count;
  const char *software_version_label;
  const char *manufacturer_label;
  const char *model_description;
  const char *default_device_label;
  const ProductDetailIds *product_detail_ids;
} ResponderDefinition;

/**
 * @brief A base implementation of a responder.
 */
typedef struct {
  /**
   * @brief The ResponderDefinition
   */
  const ResponderDefinition *def;

  char device_label[RDM_DEFAULT_STRING_SIZE];  //!< Device label
  uint8_t uid[UID_LENGTH];  //!< Responder's UID
  uint16_t dmx_start_address;  //!< DMX start address
  uint8_t queued_message_count;  //!< queued message count.
  bool is_muted;  //!< The mute state for the responder
  bool identify_on;  //!< The identify state for the responder.
  bool using_factory_defaults;  //!< True if using factory defaults.
} RDMResponder;

/**
 * @brief The global RDMResponder object.
 */
extern RDMResponder g_responder;

/**
 * @brief Indicates there is no response required for the request.
 */
static const int RDM_RESPONDER_NO_RESPONSE = 0;

/**
 * @brief Initialize an RDMResponder struct.
 * @param uid The UID to use for the responder.
 */
void RDMResponder_Initialize(const uint8_t uid[UID_LENGTH]);

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
                              RDMPid pid,
                              unsigned int param_data_length);

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
 * @brief Send a RDM NACK.
 * @param incoming_header The header of the incoming frame.
 * @param reason The NACK reason code.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_BuildNack(const RDMHeader *incoming_header,
                           RDMNackReason reason);

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

// PID Handlers
// ----------------------------------------------------------------------------

/**
 * @brief Build a response containing a string.
 * @param incoming_header The header of the incoming frame.
 * @param reply_string The string to reply with
 * @param string_size The length of the string, including the terminating NULL.
 * @returns The size of the RDM response frame.
 */
int RDMResponder_GenericReturnString(const RDMHeader *incoming_header,
                                     const char *reply_string,
                                     unsigned int string_size);

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
