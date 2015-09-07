/*
 * This is the stub for plib_nvm.h used for the tests. It contains the bare
 * minimum required to implement the mock flash symbols.
 */

#ifndef TESTS_HARMONY_INCLUDE_PERIPHERAL_NVM_PLIB_NVM_H_
#define TESTS_HARMONY_INCLUDE_PERIPHERAL_NVM_PLIB_NVM_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
  NVM_ID_0 = 0,
  NVM_NUMBER_OF_MODULES
} NVM_MODULE_ID;

typedef enum {
  WORD_PROGRAM_OPERATION = 0x1,
  ROW_PROGRAM_OPERATION = 0x3,
  PAGE_ERASE_OPERATION = 0x4,
  FLASH_ERASE_OPERATION = 0x5,
  NO_OPERATION = 0x0
} NVM_OPERATION_MODE;

void PLIB_NVM_MemoryModifyInhibit(NVM_MODULE_ID index);

void PLIB_NVM_MemoryOperationSelect(NVM_MODULE_ID index,
                                    NVM_OPERATION_MODE operationmode);

void PLIB_NVM_MemoryModifyEnable(NVM_MODULE_ID index);

void PLIB_NVM_FlashWriteKeySequence(NVM_MODULE_ID index, uint32_t keysequence);

void PLIB_NVM_FlashWriteStart(NVM_MODULE_ID index);

void PLIB_NVM_FlashAddressToModify(NVM_MODULE_ID index, uint32_t address);

void PLIB_NVM_FlashProvideData(NVM_MODULE_ID index, uint32_t data);

bool PLIB_NVM_FlashWriteCycleHasCompleted(NVM_MODULE_ID index);

bool PLIB_NVM_WriteOperationHasTerminated(NVM_MODULE_ID index);

uint32_t PLIB_NVM_FlashRead(NVM_MODULE_ID index, uint32_t address);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_HARMONY_INCLUDE_PERIPHERAL_NVM_PLIB_NVM_H_
