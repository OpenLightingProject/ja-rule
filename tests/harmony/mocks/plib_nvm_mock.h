#ifndef TESTS_HARMONY_MOCKS_PLIB_NVM_MOCK_H_
#define TESTS_HARMONY_MOCKS_PLIB_NVM_MOCK_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <gmock/gmock.h>
#include "peripheral/nvm/plib_nvm.h"

class MockNVM {
 public:
  MOCK_METHOD1(MemoryModifyInhibit, void(NVM_MODULE_ID index));
  MOCK_METHOD2(MemoryOperationSelect,
               void(NVM_MODULE_ID index, NVM_OPERATION_MODE operationmode));
  MOCK_METHOD1(MemoryModifyEnable, void(NVM_MODULE_ID index));
  MOCK_METHOD2(FlashWriteKeySequence,
               void(NVM_MODULE_ID index, uint32_t keysequence));
  MOCK_METHOD1(FlashWriteStart, void(NVM_MODULE_ID index));
  MOCK_METHOD2(FlashAddressToModify,
               void(NVM_MODULE_ID index, uint32_t address));
  MOCK_METHOD2(FlashProvideData, void(NVM_MODULE_ID index, uint32_t data));
  MOCK_METHOD1(FlashWriteCycleHasCompleted, bool(NVM_MODULE_ID index));
  MOCK_METHOD1(WriteOperationHasTerminated, bool(NVM_MODULE_ID index));
  MOCK_METHOD2(FlashRead, uint32_t(NVM_MODULE_ID index, uint32_t address));
};

void NVM_SetMock(MockNVM* mock);

#ifdef  __cplusplus
}
#endif

#endif  // TESTS_HARMONY_MOCKS_PLIB_NVM_MOCK_H_
