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
 * network_model.c
 * Copyright (C) 2015 Simon Newton
 */
#include "network_model.h"

#include <stdlib.h>

#include "constants.h"
#include "macros.h"
#include "random.h"
#include "rdm_buffer.h"
#include "rdm_frame.h"
#include "rdm_responder.h"
#include "rdm_util.h"
#include "utils.h"

// Various constants
enum { NUMBER_OF_NAMESERVERS = 3 };
enum { NUMBER_OF_INTERFACES = 3 };
enum { INTERFACE_ID_SIZE = 4 };
enum { SOFTWARE_VERSION = 0 };

typedef enum {
  CONFIG_SOURCE_STATIC = 0x00,
  CONFIG_SOURCE_DHCP = 0x01,
  CONFIG_SOURCE_ZEROCONF = 0x02,
  CONFIG_SOURCE_NONE = 0x03
} ConfigSource;

typedef enum {
  LAN_INTERFACE_ID = 1,
  IPSEC_INTERFACE_ID = 3,
  WLAN_INTERFACE_ID = 4
} InterfaceId;

static const char DEVICE_MODEL_DESCRIPTION[] = "Ja Rule Network Device";
static const char SOFTWARE_LABEL[] = "Alpha";
static const char DEFAULT_DEVICE_LABEL[] = "Ja Rule";
static const char DEFAULT_HOSTNAME[] = "responder";
static const char DEFAULT_DOMAINNAME[] = "local";
static const uint32_t DHCP_FAILURE_RATIO = 3;  // Fail 1 / n DHCP requests.

static const ResponderDefinition RESPONDER_DEFINITION;

/*
 * @brief The read only state for a network Interface.
 */
typedef struct {
  const char *label;
  uint32_t id;
  uint16_t hardware_type;
  uint8_t hardware_address[MAC_ADDRESS_SIZE];
  bool supports_dhcp;
  bool dhcp_can_fail;
} InterfaceDefinition;

/*
 * @brief The mutable state for an interface
 */
typedef struct {
  uint32_t configured_ip;
  uint32_t current_ip;
  uint8_t current_netmask;
  uint8_t configured_netmask;
  ConfigSource config_source;
  bool current_dhcp_mode;
  bool current_zeroconf_mode;
  bool configured_dhcp_mode;
  bool configured_zeroconf_mode;
} InterfaceState;

/*
 * @brief The network model state.
 */
typedef struct {
  InterfaceState *interfaces;
  unsigned int interface_count;

  uint16_t default_interface_route;
  uint32_t default_route;
  uint32_t nameservers[NUMBER_OF_NAMESERVERS];
  char hostname[DNS_HOST_NAME_SIZE];
  char domain_name[DNS_DOMAIN_NAME_SIZE];
} NetworkModel;

// Data
static const char ETHERNET_INTERFACE_NAME[] = "eth0";
static const char IPSEC_INTERFACE_NAME[] = "tun0";
static const char WIFI_INTERFACE_NAME[] = "wlan0";

static const InterfaceDefinition INTERFACE_DEFINITIONS[] = {
  {
    .label = ETHERNET_INTERFACE_NAME,
    .id = LAN_INTERFACE_ID,
    .hardware_type = ETHERNET_HARDWARE_TYPE,
    // Locally administered MAC address
    .hardware_address = {0x52, 0x12, 0x34, 0x56, 0x78, 0x9a},
    .supports_dhcp = true,
    .dhcp_can_fail = false
  },
  {
    .label = IPSEC_INTERFACE_NAME,
    .id = IPSEC_INTERFACE_ID,
    .hardware_type = IPSEC_HARDWARE_TYPE,
    // No h/w address for ptp links
    .hardware_address = {0, 0, 0, 0, 0, 0},
    .supports_dhcp = false,
    .dhcp_can_fail = false,
  },
  {
    .label = WIFI_INTERFACE_NAME,
    .id = WLAN_INTERFACE_ID,
    .hardware_type = ETHERNET_HARDWARE_TYPE,
    // Local admin MAC address
    .hardware_address = {0x52, 0xab, 0xcd, 0xef, 0x01, 0x23},
    .supports_dhcp = true,
    .dhcp_can_fail = true
  },
};

static InterfaceState g_interfaces[NUMBER_OF_INTERFACES];

static NetworkModel g_network_model;

// Helper functions
// ----------------------------------------------------------------------------

static int LookupIndex(unsigned int id) {
  unsigned int i = 0;
  for (; i < g_network_model.interface_count; i++) {
    if (INTERFACE_DEFINITIONS[i].id == id) {
      return i;
    }
  }
  return -1;
}

/*
 * @brief Simulate getting an DHCP address.
 *
 * This randomly fails and returns 0.0.0.0.
 */
static uint32_t GetDHCPAddress(bool can_fail) {
  if (can_fail && Random_PseudoGet() % DHCP_FAILURE_RATIO == 0) {
    // fail 1/3 of the time so we can test zeroconf
    return IPV4_UNCONFIGURED;
  }
  return (10 << 24) + (Random_PseudoGet() & 0xffffff);
}

/*
 * @brief Use a zeroconf address or fallback to unassigned.
 */
static void UseZeroconfOrUnassign(InterfaceState *interface) {
  if (interface->current_zeroconf_mode) {
    interface->current_ip = 0xa9fe0000 + (Random_PseudoGet() % 0xfeff);
    interface->current_netmask = 16;
    interface->config_source = CONFIG_SOURCE_ZEROCONF;
  } else {
    interface->config_source = CONFIG_SOURCE_NONE;
    interface->current_ip = IPV4_UNCONFIGURED;
    interface->current_netmask = 0;
  }
}

static void ConfigureInterface(unsigned int index) {
  InterfaceState *interface = &g_network_model.interfaces[index];
  interface->current_dhcp_mode = interface->configured_dhcp_mode;
  interface->current_zeroconf_mode = interface->configured_zeroconf_mode;

  uint32_t dhcp_address;
  if (interface->configured_ip) {
    // Static IP, use that
    interface->current_ip = interface->configured_ip;
    interface->current_netmask = interface->configured_netmask;
    interface->config_source = CONFIG_SOURCE_STATIC;
  } else if (interface->configured_dhcp_mode &&
             (dhcp_address =
              GetDHCPAddress(INTERFACE_DEFINITIONS[index].dhcp_can_fail))) {
    interface->current_ip = dhcp_address;
    interface->current_netmask = 8;
    interface->config_source = CONFIG_SOURCE_DHCP;
  } else {
    UseZeroconfOrUnassign(interface);
  }
}

// PID Handlers
// ----------------------------------------------------------------------------
int NetworkModel_GetListInterfaces(const RDMHeader *header,
                                   UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  unsigned int i = 0;
  for (; i < g_network_model.interface_count; i++) {
    ptr = PushUInt32(ptr, INTERFACE_DEFINITIONS[i].id);
    ptr = PushUInt16(ptr, INTERFACE_DEFINITIONS[i].hardware_type);
  }
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int NetworkModel_GetInterfaceLabel(const RDMHeader *header,
                                   const uint8_t *param_data) {
  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  unsigned int offset = sizeof(RDMHeader);
  memcpy(g_rdm_buffer + offset, param_data, INTERFACE_ID_SIZE);
  offset += INTERFACE_ID_SIZE;
  offset += RDMUtil_StringCopy((char*) (g_rdm_buffer + offset),
                               RDM_DEFAULT_STRING_SIZE,
                               INTERFACE_DEFINITIONS[index].label,
                               RDM_DEFAULT_STRING_SIZE);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, offset);
}

int NetworkModel_GetHardwareAddress(const RDMHeader *header,
                                    const uint8_t *param_data) {
  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  if (INTERFACE_DEFINITIONS[index].hardware_type != ETHERNET_HARDWARE_TYPE) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  unsigned int offset = sizeof(RDMHeader);
  memcpy(g_rdm_buffer + offset, param_data, INTERFACE_ID_SIZE);
  offset += INTERFACE_ID_SIZE;
  memcpy(g_rdm_buffer + offset, INTERFACE_DEFINITIONS[index].hardware_address,
         MAC_ADDRESS_SIZE);
  offset += MAC_ADDRESS_SIZE;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, offset);
}

int NetworkModel_GetDHCPMode(const RDMHeader *header,
                             const uint8_t *param_data) {
  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  unsigned int offset = sizeof(RDMHeader);
  memcpy(g_rdm_buffer + offset, param_data, INTERFACE_ID_SIZE);
  offset += INTERFACE_ID_SIZE;
  g_rdm_buffer[offset++] =
      g_network_model.interfaces[index].configured_dhcp_mode;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, offset);
}

int NetworkModel_SetDHCPMode(const RDMHeader *header,
                             const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint32_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  if (!INTERFACE_DEFINITIONS[index].supports_dhcp) {
    return RDMResponder_BuildNack(header, NR_ACTION_NOT_SUPPORTED);
  }

  g_network_model.interfaces[index].configured_dhcp_mode =
      param_data[INTERFACE_ID_SIZE];

  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_GetZeroconfMode(const RDMHeader *header,
                                 const uint8_t *param_data) {
  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  unsigned int offset = sizeof(RDMHeader);
  memcpy(g_rdm_buffer + offset, param_data, INTERFACE_ID_SIZE);
  offset += INTERFACE_ID_SIZE;
  g_rdm_buffer[offset++] =
      g_network_model.interfaces[index].configured_zeroconf_mode;
  return RDMResponder_AddHeaderAndChecksum(header, ACK, offset);
}

int NetworkModel_SetZeroconfMode(const RDMHeader *header,
                                 const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint32_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  if (!INTERFACE_DEFINITIONS[index].supports_dhcp) {
    return RDMResponder_BuildNack(header, NR_ACTION_NOT_SUPPORTED);
  }

  g_network_model.interfaces[index].configured_zeroconf_mode =
      param_data[INTERFACE_ID_SIZE];

  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_GetCurrentAddress(const RDMHeader *header,
                                   const uint8_t *param_data) {
  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  InterfaceState *interface = &g_network_model.interfaces[index];

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  memcpy(ptr, param_data, INTERFACE_ID_SIZE);
  ptr += INTERFACE_ID_SIZE;
  ptr = PushUInt32(ptr, interface->current_ip);
  *ptr++ = interface->current_netmask;
  if (!INTERFACE_DEFINITIONS[index].supports_dhcp) {
    *ptr++ = DHCP_STATUS_INACTIVE;
  } else {
    *ptr++ = (interface->config_source == CONFIG_SOURCE_DHCP ?
              DHCP_STATUS_ACTIVE : DHCP_STATUS_INACTIVE);
  }

  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int NetworkModel_GetStaticAddress(const RDMHeader *header,
                                  const uint8_t *param_data) {
  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  InterfaceState *interface = &g_network_model.interfaces[index];


  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  memcpy(ptr, param_data, INTERFACE_ID_SIZE);
  ptr += INTERFACE_ID_SIZE;
  ptr = PushUInt32(ptr, interface->configured_ip);
  *ptr++ = interface->configured_netmask;

  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int NetworkModel_SetStaticAddress(const RDMHeader *header,
                                  const uint8_t *param_data) {
  if (header->param_data_length != 2 * sizeof(uint32_t) + sizeof(uint8_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  const uint32_t ip = ExtractUInt32(&param_data[4]);
  const uint8_t netmask = param_data[8];

  if (netmask > MAX_NETMASK) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_network_model.interfaces[index].configured_ip = ip;
  g_network_model.interfaces[index].configured_netmask = netmask;

  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_RenewDHCP(const RDMHeader *header,
                           const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint32_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  InterfaceState *interface = &g_network_model.interfaces[index];
  if (interface->config_source == CONFIG_SOURCE_STATIC ||
      interface->current_dhcp_mode == false) {
    return RDMResponder_BuildNack(header, NR_ACTION_NOT_SUPPORTED);
  }

  if (interface->config_source == CONFIG_SOURCE_DHCP) {
    if (Random_PseudoGet() % DHCP_FAILURE_RATIO == 0) {
      UseZeroconfOrUnassign(interface);
    }
  } else {
    uint32_t dhcp_address = GetDHCPAddress(
        INTERFACE_DEFINITIONS[index].dhcp_can_fail);
    if (dhcp_address) {
      interface->current_ip = dhcp_address;
      interface->current_netmask = 8;
      interface->config_source = CONFIG_SOURCE_DHCP;
    } else {
      UseZeroconfOrUnassign(interface);
    }
  }

  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_ReleaseDHCP(const RDMHeader *header,
                             const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint32_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  InterfaceState *interface = &g_network_model.interfaces[index];
  if (interface->config_source != CONFIG_SOURCE_DHCP) {
    return RDMResponder_BuildNack(header, NR_ACTION_NOT_SUPPORTED);
  }

  UseZeroconfOrUnassign(interface);
  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_ApplyInterfaceConfiguration(const RDMHeader *header,
                                             const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint32_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const int index = LookupIndex(ExtractUInt32(param_data));
  if (index < 0) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  ConfigureInterface(index);
  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_GetDefaultRoute(const RDMHeader *header,
                                 UNUSED const uint8_t *param_data) {
  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  ptr = PushUInt32(ptr, g_network_model.default_interface_route);
  ptr = PushUInt32(ptr, g_network_model.default_route);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int NetworkModel_SetDefaultRoute(const RDMHeader *header,
                                 const uint8_t *param_data) {
  if (header->param_data_length != 2 * sizeof(uint32_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint32_t interface_id = ExtractUInt32(param_data);
  const uint32_t ip = ExtractUInt32(&param_data[4]);

  if (interface_id != NO_DEFAULT_ROUTE && ip != NO_DEFAULT_ROUTE) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  if (interface_id != NO_DEFAULT_ROUTE &&
      (LookupIndex(interface_id) < 0 || interface_id != IPSEC_INTERFACE_ID)) {
    // Only the ptp IPsec interface can be used as the default route
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }
  g_network_model.default_interface_route = interface_id;
  g_network_model.default_route = ip;
  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_GetNameServer(const RDMHeader *header,
                               const uint8_t *param_data) {
  const uint8_t index = param_data[0];
  if (index >= NUMBER_OF_NAMESERVERS) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  uint8_t *ptr = g_rdm_buffer + sizeof(RDMHeader);
  *ptr++ = index;
  ptr = PushUInt32(ptr, g_network_model.nameservers[index]);
  return RDMResponder_AddHeaderAndChecksum(header, ACK, ptr - g_rdm_buffer);
}

int NetworkModel_SetNameServer(const RDMHeader *header,
                               const uint8_t *param_data) {
  if (header->param_data_length != sizeof(uint8_t) + sizeof(uint32_t)) {
    return RDMResponder_BuildNack(header, NR_FORMAT_ERROR);
  }

  const uint8_t index = param_data[0];
  if (index >= NUMBER_OF_NAMESERVERS) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  g_network_model.nameservers[index] = ExtractUInt32(&param_data[1]);
  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_GetHostname(const RDMHeader *header,
                             UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericReturnString(header, g_network_model.hostname,
                                          DNS_HOST_NAME_SIZE);
}

int NetworkModel_SetHostname(const RDMHeader *header,
                             const uint8_t *param_data) {
  if (header->param_data_length == 0 ||
      header->param_data_length > DNS_HOST_NAME_SIZE) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  RDMUtil_StringCopy(g_network_model.hostname, DNS_HOST_NAME_SIZE,
                     (const char*) param_data,
                     header->param_data_length);
  return RDMResponder_BuildSetAck(header);
}

int NetworkModel_GetDomainName(const RDMHeader *header,
                               UNUSED const uint8_t *param_data) {
  return RDMResponder_GenericReturnString(header, g_network_model.domain_name,
                                          DNS_DOMAIN_NAME_SIZE);
}

int NetworkModel_SetDomainName(const RDMHeader *header,
                               const uint8_t *param_data) {
  if (header->param_data_length > DNS_DOMAIN_NAME_SIZE) {
    return RDMResponder_BuildNack(header, NR_DATA_OUT_OF_RANGE);
  }

  RDMUtil_StringCopy(g_network_model.domain_name, DNS_DOMAIN_NAME_SIZE,
                     (const char*) param_data,
                     header->param_data_length);
  return RDMResponder_BuildSetAck(header);
}

// Public Functions
// ----------------------------------------------------------------------------
void NetworkModel_Initialize() {
  // Initialize the InterfaceState array to something interesting.

  // eth0 is 192.168.0.1/24
  g_interfaces[0].configured_dhcp_mode = false;
  g_interfaces[0].configured_zeroconf_mode = false;
  g_interfaces[0].configured_ip = 0xc0a80001;
  g_interfaces[0].configured_netmask = 24;

  // IPSEC is 10.1.1.1/31
  g_interfaces[1].configured_dhcp_mode = false;
  g_interfaces[1].configured_zeroconf_mode = false;
  g_interfaces[1].configured_ip = 167837953;
  g_interfaces[1].configured_netmask = 31;

  // WLAN iface is DHCP
  g_interfaces[2].configured_dhcp_mode = true;
  g_interfaces[2].configured_zeroconf_mode = true;
  g_interfaces[2].configured_ip = IPV4_UNCONFIGURED;
  g_interfaces[2].configured_netmask = 0;

  g_network_model.interface_count = NUMBER_OF_INTERFACES;
  g_network_model.interfaces = g_interfaces;

  unsigned int i = 0;
  for (; i < g_network_model.interface_count; i++) {
    ConfigureInterface(i);
  }

  g_network_model.default_interface_route = NO_DEFAULT_ROUTE;
  g_network_model.default_route = NO_DEFAULT_ROUTE;
  for (i = 0; i < NUMBER_OF_NAMESERVERS; i++) {
    g_network_model.nameservers[i] = IPV4_UNCONFIGURED;
  }
  RDMUtil_StringCopy(g_network_model.hostname, DNS_HOST_NAME_SIZE,
                    DEFAULT_HOSTNAME, DNS_HOST_NAME_SIZE);
  RDMUtil_StringCopy(g_network_model.domain_name, DNS_DOMAIN_NAME_SIZE,
                     DEFAULT_DOMAINNAME, DNS_DOMAIN_NAME_SIZE);
}

static void NetworkModel_Activate() {
  g_responder->def = &RESPONDER_DEFINITION;
  RDMResponder_ResetToFactoryDefaults();
}

static void NetworkModel_Deactivate() {}

static int NetworkModel_Ioctl(ModelIoctl command, uint8_t *data,
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

static int NetworkModel_HandleRequest(const RDMHeader *header,
                                     const uint8_t *param_data) {
  if (!RDMUtil_RequiresAction(g_responder->uid, header->dest_uid)) {
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

static void NetworkModel_Tasks() {}

const ModelEntry NETWORK_MODEL_ENTRY = {
  .model_id = NETWORK_MODEL_ID,
  .activate_fn = NetworkModel_Activate,
  .deactivate_fn = NetworkModel_Deactivate,
  .ioctl_fn = NetworkModel_Ioctl,
  .request_fn = NetworkModel_HandleRequest,
  .tasks_fn = NetworkModel_Tasks
};

static const PIDDescriptor PID_DESCRIPTORS[] = {
  {PID_SUPPORTED_PARAMETERS, RDMResponder_GetSupportedParameters, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_INFO, RDMResponder_GetDeviceInfo, 0, (PIDCommandHandler) NULL},
  {PID_PRODUCT_DETAIL_ID_LIST, RDMResponder_GetProductDetailIds, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_MODEL_DESCRIPTION, RDMResponder_GetDeviceModelDescription, 0,
    (PIDCommandHandler) NULL},
  {PID_MANUFACTURER_LABEL, RDMResponder_GetManufacturerLabel, 0,
    (PIDCommandHandler) NULL},
  {PID_DEVICE_LABEL, RDMResponder_GetDeviceLabel, 0,
    RDMResponder_SetDeviceLabel},
  {PID_SOFTWARE_VERSION_LABEL, RDMResponder_GetSoftwareVersionLabel, 0,
    (PIDCommandHandler) NULL},
  {PID_IDENTIFY_DEVICE, RDMResponder_GetIdentifyDevice, 0,
    RDMResponder_SetIdentifyDevice},
  {PID_LIST_INTERFACES, NetworkModel_GetListInterfaces, 0,
    (PIDCommandHandler) NULL},
  {PID_INTERFACE_LABEL, NetworkModel_GetInterfaceLabel, 4,
    (PIDCommandHandler) NULL},
  {PID_INTERFACE_HARDWARE_ADDRESS_TYPE1, NetworkModel_GetHardwareAddress, 4,
    (PIDCommandHandler) NULL},
  {PID_IPV4_DHCP_MODE, NetworkModel_GetDHCPMode, 4, NetworkModel_SetDHCPMode},
  {PID_IPV4_ZEROCONF_MODE, NetworkModel_GetZeroconfMode, 4,
    NetworkModel_SetZeroconfMode},
  {PID_IPV4_CURRENT_ADDRESS, NetworkModel_GetCurrentAddress, 4,
    (PIDCommandHandler) NULL},
  {PID_IPV4_STATIC_ADDRESS, NetworkModel_GetStaticAddress, 4,
    NetworkModel_SetStaticAddress},
  {PID_INTERFACE_RENEW_DHCP, (PIDCommandHandler) NULL, 0,
    NetworkModel_RenewDHCP},
  {PID_INTERFACE_RELEASE_DHCP, (PIDCommandHandler) NULL, 0,
    NetworkModel_ReleaseDHCP},
  {PID_INTERFACE_APPLY_CONFIGURATION, (PIDCommandHandler) NULL, 4,
    NetworkModel_ApplyInterfaceConfiguration},
  {PID_IPV4_DEFAULT_ROUTE, NetworkModel_GetDefaultRoute, 0,
    NetworkModel_SetDefaultRoute},
  {PID_DNS_NAME_SERVER, NetworkModel_GetNameServer, 1,
    NetworkModel_SetNameServer},
  {PID_DNS_HOSTNAME, NetworkModel_GetHostname, 0, NetworkModel_SetHostname},
  {PID_DNS_DOMAIN_NAME, NetworkModel_GetDomainName, 0,
    NetworkModel_SetDomainName},
};

static const ProductDetailIds PRODUCT_DETAIL_ID_LIST = {
  .ids = {PRODUCT_DETAIL_TEST, PRODUCT_DETAIL_CHANGEOVER_MANUAL,
          PRODUCT_DETAIL_ROUTER},
  .size = 3
};

static const ResponderDefinition RESPONDER_DEFINITION = {
  .descriptors = PID_DESCRIPTORS,
  .descriptor_count = sizeof(PID_DESCRIPTORS) / sizeof(PIDDescriptor),
  .sensors = NULL,
  .sensor_count = 0,
  .personalities = NULL,
  .personality_count = 0,
  .software_version_label = SOFTWARE_LABEL,
  .manufacturer_label = MANUFACTURER_LABEL,
  .model_description = DEVICE_MODEL_DESCRIPTION,
  .product_detail_ids = &PRODUCT_DETAIL_ID_LIST,
  .default_device_label = DEFAULT_DEVICE_LABEL,
  .software_version = SOFTWARE_VERSION,
  .model_id = NETWORK_MODEL_ID,
  .product_category = PRODUCT_CATEGORY_TEST_EQUIPMENT
};
