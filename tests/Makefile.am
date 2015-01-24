
# As suggested by libtoolize
ACLOCAL_AMFLAGS = -I config

include aminclude.am

AM_CFLAGS = -Wall -Werror

WARNING_CFLAGS = -Wall -Wformat -W -Werror
WARNING_CXXFLAGS = $(WARNING_CFLAGS) -fvisibility-inlines-hidden

# LIBRARIES
##################################################

TEST_FLAGS = -I include $(WARNING_CXXFLAGS)

noinst_LTLIBRARIES = lib/libtest.la

lib_libtest_la_SOURCES = include/TestUtils.h \
                         include/TestUtilsPrivate.h \
                         lib/TestMain.cpp \
                         lib/TestUtils.cpp
lib_libtest_la_CXXFLAGS = $(TEST_FLAGS)

# Code Under Test
##################################################

BUILD_FLAGS = -I system_config -I ../src $(WARNING_CFLAGS)

noinst_LTLIBRARIES += src/liblogger.la \
                      src/libstreamdecoder.la

src_liblogger_la_SOURCES = src/logger.c
src_liblogger_la_CFLAGS = $(BUILD_FLAGS)

src_libstreamdecoder_la_SOURCES = src/stream_decoder.c
src_libstreamdecoder_la_CFLAGS = $(BUILD_FLAGS)

# TESTS
################################################
TESTING_FLAGS = -I system_config -I ../src -I include \
                -std=c++11 \
                $(WARNING_CXXFLAGS) $(CPPUNIT_CFLAGS)

TESTING_LIBS = $(CPPUNIT_LIBS) \
               lib/libtest.la

TESTS = tests/logger_tester \
        tests/stream_decoder_tester

check_PROGRAMS = $(TESTS)

tests_logger_tester_SOURCES = tests/LoggerTest.cpp
tests_logger_tester_CXXFLAGS = $(TESTING_FLAGS)
tests_logger_tester_LDADD = $(TESTING_LIBS) \
                            src/liblogger.la

tests_stream_decoder_tester_SOURCES = tests/StreamDecoderTest.cpp
tests_stream_decoder_tester_CXXFLAGS = $(TESTING_FLAGS)
tests_stream_decoder_tester_LDADD = $(TESTING_LIBS) \
                                   src/libstreamdecoder.la
