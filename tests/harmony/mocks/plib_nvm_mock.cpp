#include <gmock/gmock.h>
#include "plib_nvm_mock.h"

namespace {
  MockNVM *g_nvm_mock = NULL;
}

void PLIB_NVM_MemoryModifyInhibit(NVM_MODULE_ID index) {
  if (g_nvm_mock) {
    g_nvm_mock->MemoryModifyInhibit(index);
  }
}

void PLIB_NVM_MemoryOperationSelect(NVM_MODULE_ID index,
                                    NVM_OPERATION_MODE operationmode) {
  if (g_nvm_mock) {
    g_nvm_mock->MemoryOperationSelect(index, operationmode);
  }
}

void PLIB_NVM_MemoryModifyEnable(NVM_MODULE_ID index) {
  if (g_nvm_mock) {
    g_nvm_mock->MemoryModifyEnable(index);
  }
}

void PLIB_NVM_FlashWriteKeySequence(NVM_MODULE_ID index, uint32_t keysequence) {
  if (g_nvm_mock) {
    g_nvm_mock->FlashWriteKeySequence(index, keysequence);
  }
}

void PLIB_NVM_FlashWriteStart(NVM_MODULE_ID index) {
  if (g_nvm_mock) {
    g_nvm_mock->FlashWriteStart(index);
  }
}

void PLIB_NVM_FlashAddressToModify(NVM_MODULE_ID index, uint32_t address) {
  if (g_nvm_mock) {
    g_nvm_mock->FlashAddressToModify(index, address);
  }
}

void PLIB_NVM_FlashProvideData(NVM_MODULE_ID index, uint32_t data) {
  if (g_nvm_mock) {
    g_nvm_mock->FlashProvideData(index, data);
  }
}

bool PLIB_NVM_FlashWriteCycleHasCompleted(NVM_MODULE_ID index) {
  if (g_nvm_mock) {
    return g_nvm_mock->FlashWriteCycleHasCompleted(index);
  }
  return true;
}

bool PLIB_NVM_WriteOperationHasTerminated(NVM_MODULE_ID index) {
  if (g_nvm_mock) {
    return g_nvm_mock->WriteOperationHasTerminated(index);
  }
  return true;
}

uint32_t PLIB_NVM_FlashRead(NVM_MODULE_ID index, uint32_t address) {
  if (g_nvm_mock) {
    return g_nvm_mock->FlashRead(index, address);
  }
  return 0;
}
