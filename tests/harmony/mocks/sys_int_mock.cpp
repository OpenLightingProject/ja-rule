#include <gmock/gmock.h>
#include "sys_int_mock.h"

namespace {
  MockSysInt *g_sys_int_mock = NULL;
}

void SYS_INT_SetMock(MockSysInt* mock) {
  g_sys_int_mock = mock;
}

bool SYS_INT_SourceStatusGet(INT_SOURCE source) {
  if (g_sys_int_mock) {
    return g_sys_int_mock->SourceStatusGet(source);
  }
  return false;
}

void SYS_INT_SourceStatusClear(INT_SOURCE source) {
  if (g_sys_int_mock) {
    g_sys_int_mock->SourceStatusClear(source);
  }
}

void SYS_INT_SourceEnable(INT_SOURCE source) {
  if (g_sys_int_mock) {
    g_sys_int_mock->SourceEnable(source);
  }
}

bool SYS_INT_SourceDisable(INT_SOURCE source) {
  if (g_sys_int_mock) {
    return g_sys_int_mock->SourceDisable(source);
  }
  return false;
}

void SYS_INT_VectorPrioritySet(INT_VECTOR vector, INT_PRIORITY_LEVEL priority) {
  if (g_sys_int_mock) {
    g_sys_int_mock->VectorPrioritySet(vector, priority);
  }
}

void SYS_INT_VectorSubprioritySet(INT_VECTOR vector,
                                  INT_SUBPRIORITY_LEVEL subpriority) {
  if (g_sys_int_mock) {
    g_sys_int_mock->VectorSubprioritySet(vector, subpriority);
  }
}

