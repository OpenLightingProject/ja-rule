#ifndef TESTS_HARMONY_MOCKS_SYS_INT_MOCK_H_
#define TESTS_HARMONY_MOCKS_SYS_INT_MOCK_H_

#include <gmock/gmock.h>
#include "system/int/sys_int.h"

class MockSysInt {
 public:
  MOCK_METHOD1(SourceStatusGet, bool(INT_SOURCE source));
  MOCK_METHOD1(SourceStatusClear, void(INT_SOURCE source));
  MOCK_METHOD1(SourceEnable, void(INT_SOURCE source));
  MOCK_METHOD1(SourceDisable, bool(INT_SOURCE source));
  MOCK_METHOD2(VectorPrioritySet,
               void(INT_VECTOR vector, INT_PRIORITY_LEVEL priority));
  MOCK_METHOD2(VectorSubprioritySet,
               void(INT_VECTOR vector, INT_SUBPRIORITY_LEVEL subpriority));
};

void SYS_INT_SetMock(MockSysInt* mock);

#endif  // TESTS_HARMONY_MOCKS_SYS_INT_MOCK_H_
