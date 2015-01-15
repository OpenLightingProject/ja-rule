
# As suggested by libtoolize
ACLOCAL_AMFLAGS = -I config

include aminclude.am

AM_CFLAGS = -Wall -Werror

COMMON_CFLAGS = -I ../src -Wall -Wformat -W -fvisibility-inlines-hidden \
                -Werror
COMMON_CXXFLAGS = -I ../src -I include/ \
                  -Wall -Wformat -W -fvisibility-inlines-hidden

COMMON_TESTING_FLAGS = $(COMMON_CXXFLAGS) $(CPPUNIT_CFLAGS)

COMMON_TESTING_LIBS = $(CPPUNIT_LIBS) \
                      lib/libtest.la

# LIBRARIES
##################################################
noinst_LTLIBRARIES = lib/libtest.la

lib_libtest_la_SOURCES = include/TestUtils.h \
                         include/TestUtilsPrivate.h \
                         lib/TestMain.cpp \
                         lib/TestUtils.cpp
lib_libtest_la_CXXFLAGS = $(COMMON_CXXFLAGS)

# Code Under Test
##################################################
noinst_LTLIBRARIES += src/liblogger.la
src_liblogger_la_SOURCES = src/logger.c
src_liblogger_la_CFLAGS = $(COMMON_CFLAGS)

# TESTS
################################################
TESTS = tests/logger_tester

check_PROGRAMS = tests/logger_tester

tests_logger_tester_SOURCES = tests/LoggerTest.cpp
tests_logger_tester_CXXFLAGS = $(COMMON_TESTING_FLAGS)
tests_logger_tester_LDADD = $(COMMON_TESTING_LIBS) \
                            src/liblogger.la
