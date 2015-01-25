
# As suggested by libtoolize
ACLOCAL_AMFLAGS = -I config

include aminclude.am

AM_CFLAGS = -Wall -Werror

WARNING_CFLAGS = -Wall -Wformat -W -Werror
WARNING_CXXFLAGS = -fvisibility-inlines-hidden

# LIBRARIES
##################################################

TEST_FLAGS = -I include $(WARNING_CFLAGS) $(WARNING_CXXFLAGS)

noinst_LTLIBRARIES = lib/libtest.la

lib_libtest_la_SOURCES = include/TestUtils.h \
                         include/TestUtilsPrivate.h \
                         lib/TestMain.cpp \
                         lib/TestUtils.cpp
lib_libtest_la_CXXFLAGS = $(TEST_FLAGS)

# Code Under Test
##################################################

BUILD_FLAGS = -I system_config -I ../src $(WARNING_CFLAGS)

noinst_LTLIBRARIES += src/libflags.la \
                      src/liblogger.la \
                      src/libstreamdecoder.la \
                      src/libusbtransport.la

src_libflags_la_SOURCES = src/flags.c
src_libflags_la_CFLAGS = $(BUILD_FLAGS)

src_liblogger_la_SOURCES = src/logger.c
src_liblogger_la_CFLAGS = $(BUILD_FLAGS)

src_libstreamdecoder_la_SOURCES = src/stream_decoder.c
src_libstreamdecoder_la_CFLAGS = $(BUILD_FLAGS)

src_libusbtransport_la_SOURCES = src/usb_transport.c
src_libusbtransport_la_CFLAGS = $(BUILD_FLAGS)

# TESTS
################################################
TESTING_CFLAGS = -I system_config -I ../src -I include \
                 $(WARNING_CFLAGS) $(CPPUNIT_CFLAGS)

TESTING_CXXFLAGS = $(TESTING_CFLAGS) -std=c++11 $(WARNING_CXXFLAGS)

TESTING_LIBS = $(CPPUNIT_LIBS) \
               lib/libtest.la

TESTS = tests/flags_tester \
        tests/logger_tester \
        tests/stream_decoder_tester \
        tests/usb_transport_tester

check_PROGRAMS = $(TESTS)

tests_flags_tester_SOURCES = tests/FlagsTest.cpp
tests_flags_tester_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_flags_tester_LDADD = $(TESTING_LIBS) \
                            src/libflags.la

tests_logger_tester_SOURCES = tests/LoggerTest.cpp
tests_logger_tester_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_logger_tester_LDADD = $(TESTING_LIBS) \
                            src/liblogger.la

tests_stream_decoder_tester_SOURCES = tests/StreamDecoderTest.cpp
tests_stream_decoder_tester_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_stream_decoder_tester_LDADD = $(TESTING_LIBS) \
                                   src/libstreamdecoder.la

tests_usb_transport_tester_SOURCES = tests/LoggerMock.cpp \
                                     tests/USBMock.cpp \
                                     tests/USBTransportTest.cpp
tests_usb_transport_tester_CXXFLAGS = $(TESTING_CXXFLAGS) $(CMOCKA_FLAGS)
tests_usb_transport_tester_LDADD = $(CMOCKA_LIBS) \
                                   src/libusbtransport.la \
                                   src/libflags.la
