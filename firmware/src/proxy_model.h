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
 * proxy_model.h
 * Copyright (C) 2015 Simon Newton
 */

/**
 * @addtogroup rdm_models
 * @{
 * @file proxy_model.h
 * @brief An RDM Model for a proxy.
 *
 * This model simulates a proxy with two responders (children) behind it, this
 * is a fairly typical setup when wireless DMX equipment is used.
 *
 * The proxy will ACK_TIMER any requests sent to the child devices. The
 * responses can then be fetched by sendind a GET QUEUED_MESSAGE to the
 * appropriate child device.
 *
 * Only a single response per-device will by buffered, if other requests are
 * sent before a GET QUEUED_MESSAGE has been issued, the proxy will respond
 * with NR_PROXY_BUFFER_FULL.
 *
 * The last message can be retrieved with GET QUEUED_MESSAGE
 * (STATUS_GET_LAST_MESSAGE).
 */

#ifndef FIRMWARE_SRC_PROXY_MODEL_H_
#define FIRMWARE_SRC_PROXY_MODEL_H_

#include "rdm_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The ModelEntry for the Proxy model.
 */
extern const ModelEntry PROXY_MODEL_ENTRY;

/**
 * @brief Initialize the proxy model.
 */
void ProxyModel_Initialize();

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif  // FIRMWARE_SRC_PROXY_MODEL_H_
