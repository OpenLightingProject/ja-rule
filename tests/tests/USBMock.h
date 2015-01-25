/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * USBMock.h
 * Mock of the Harmony USB module.
 * Copyright (C) 2015 Simon Newton
 */

#ifndef TESTS_TESTS_USBMOCK_H_
#define TESTS_TESTS_USBMOCK_H_

#include "usb_stub.h"

typedef void (*USBEventHandler)(USB_DEVICE_EVENT, void*, uintptr_t);

#endif  // TESTS_TESTS_USBMOCK_H_
