
# As suggested by libtoolize
ACLOCAL_AMFLAGS = -I config

include aminclude.am

AM_CFLAGS = -Wall -Werror

WARNING_FLAGS = -Wall -Wformat -W -fvisibility-inlines-hidden -Werror

# LIBRARIES
##################################################

TEST_FLAGS = -I include $(WARNING_FLAGS)

noinst_LTLIBRARIES = lib/libtest.la

lib_libtest_la_SOURCES = include/TestUtils.h \
                         include/TestUtilsPrivate.h \
                         lib/TestMain.cpp \
                         lib/TestUtils.cpp
lib_libtest_la_CXXFLAGS = $(TEST_FLAGS)

# Code Under Test
##################################################

BUILD_FLAGS = -I system_config -I ../src $(WARNING_FLAGS)

noinst_LTLIBRARIES += src/liblogger.la
src_liblogger_la_SOURCES = src/logger.c
src_liblogger_la_CFLAGS = $(BUILD_FLAGS)

# TESTS
################################################
TESTING_FLAGS = -I system_config -I ../src -I include \
                $(WARNING_FLAGS) $(CPPUNIT_CFLAGS)

TESTING_LIBS = $(CPPUNIT_LIBS) \
               lib/libtest.la

TESTS = tests/logger_tester

check_PROGRAMS = tests/logger_tester

tests_logger_tester_SOURCES = tests/LoggerTest.cpp
tests_logger_tester_CXXFLAGS = $(TESTING_FLAGS)
tests_logger_tester_LDADD = $(TESTING_LIBS) \
                            src/liblogger.la
