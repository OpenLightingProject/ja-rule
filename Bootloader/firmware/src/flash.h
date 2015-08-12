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
 * flash.h
 * Copyright (C) 2015 Simon Newton
 */

#ifndef BOOTLOADER_FIRMWARE_SRC_FLASH_H_
#define BOOTLOADER_FIRMWARE_SRC_FLASH_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @{
 * @file flash.h
 * @brief Flash memory operations.
 *
 * This file defines constants that are not expected to change.
 */

/**
 * @brief Erase a page of flash memory and block until the operation is
 *   complete.
 * @param address The virtual address of the page to erase.
 * @returns true if the page was erased, false if there was an error.
 *
 * The address must be aligned correctly. On the pic32 5xx/6xx/7xx platform,
 * the address must be aligned to a 4k address.
 */
bool Flash_ErasePage(void *address);

/**
 * @brief Write a word (32-bits) to flash memory and block until the operation
 *   is complete.
 * @param address The virtual address to write to, must be 4-byte aligned.
 * @param data The value to write.
 * @returns true if the write succeeded, false if it failed.
 *
 * This page that this word belongs to must have been erased before this
 * function is called.
 */
bool Flash_WriteWord(void *address, uint32_t data);

#endif  // BOOTLOADER_FIRMWARE_SRC_FLASH_H_
