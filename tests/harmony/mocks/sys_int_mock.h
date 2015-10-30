#ifndef TESTS_HARMONY_MOCKS_SYS_INT_MOCK_H_
#define TESTS_HARMONY_MOCKS_SYS_INT_MOCK_H_

#include <gmock/gmock.h>
#include "system/int/sys_int.h"

class SysIntInterface {
 public:
  virtual ~SysIntInterface() {}

  virtual bool SourceStatusGet(INT_SOURCE source) = 0;
  virtual void SourceStatusClear(INT_SOURCE source) = 0;
  virtual void SourceEnable(INT_SOURCE source) = 0;
  virtual bool SourceDisable(INT_SOURCE source) = 0;
  virtual void VectorPrioritySet(INT_VECTOR vector,
                                 INT_PRIORITY_LEVEL priority) = 0;
  virtual void VectorSubprioritySet(INT_VECTOR vector,
                                    INT_SUBPRIORITY_LEVEL subpriority) = 0;
};

class MockSysInt : public SysIntInterface {
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

void SYS_INT_SetMock(SysIntInterface* mock);

#endif  // TESTS_HARMONY_MOCKS_SYS_INT_MOCK_H_
