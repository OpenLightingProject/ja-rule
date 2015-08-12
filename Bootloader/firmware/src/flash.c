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
 * flash.c
 * Copyright (C) 2015 Simon Newton
 */

#include "flash.h"
#include "peripheral/nvm/plib_nvm.h"
#include <sys/kmem.h>

enum { NVM_PROGRAM_UNLOCK_KEY1 = 0xAA996655 };
enum { NVM_PROGRAM_UNLOCK_KEY2 = 0x556699AA };

static void PerformOperation(uint32_t nvmop) {
  // Disable flash write/erase operations
  PLIB_NVM_MemoryModifyInhibit(NVM_ID_0);

  PLIB_NVM_MemoryOperationSelect(NVM_ID_0, nvmop);

  // Allow memory modifications
  PLIB_NVM_MemoryModifyEnable(NVM_ID_0);

  /* Unlock the Flash */
  PLIB_NVM_FlashWriteKeySequence(NVM_ID_0, 0);
  PLIB_NVM_FlashWriteKeySequence(NVM_ID_0, NVM_PROGRAM_UNLOCK_KEY1);
  PLIB_NVM_FlashWriteKeySequence(NVM_ID_0, NVM_PROGRAM_UNLOCK_KEY2);

  PLIB_NVM_FlashWriteStart(NVM_ID_0);
}

bool Flash_ErasePage(void *address) {
  PLIB_NVM_FlashAddressToModify(NVM_ID_0, KVA_TO_PA(address));
  PerformOperation(PAGE_ERASE_OPERATION);

  while (!PLIB_NVM_FlashWriteCycleHasCompleted(NVM_ID_0)) {
    ;
  }

  return !PLIB_NVM_WriteOperationHasTerminated(NVM_ID_0);
}

bool Flash_WriteWord(void *address, uint32_t data) {
  PLIB_NVM_FlashAddressToModify(NVM_ID_0, KVA_TO_PA(address));
  PLIB_NVM_FlashProvideData(NVM_ID_0, data);
  PerformOperation(WORD_PROGRAM_OPERATION);

  while (!PLIB_NVM_FlashWriteCycleHasCompleted(NVM_ID_0)) {
    ;
  }

  return !PLIB_NVM_WriteOperationHasTerminated(NVM_ID_0);
}
