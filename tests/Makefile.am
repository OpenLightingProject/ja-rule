
# As suggested by libtoolize
ACLOCAL_AMFLAGS = -I config

include aminclude.am

AM_CFLAGS = -Wall -Werror

WARNING_CFLAGS = -Wall -Wformat -W -Werror
WARNING_CXXFLAGS = -fvisibility-inlines-hidden

# Code Under Test
##################################################

BUILD_FLAGS = -I system_config -I harmony/include -I ../firmware/src $(WARNING_CFLAGS)

noinst_LTLIBRARIES = harmony/mocks/libharmonymock.la \
                     src/libflags.la \
                     src/liblogger.la \
                     src/libmessagehandler.la \
                     src/libstreamdecoder.la \
                     src/libtransceiver.la \
                     src/libusbtransport.la

harmony_mocks_libharmonymock_la_SOURCES = \
    harmony/mocks/plib_ports_mock.cpp \
    harmony/mocks/plib_ports_mock.h \
    harmony/mocks/plib_tmr_mock.cpp \
    harmony/mocks/plib_tmr_mock.h \
    harmony/mocks/plib_usart_mock.cpp \
    harmony/mocks/plib_usart_mock.h \
    harmony/mocks/sys_clk_mock.cpp \
    harmony/mocks/sys_clk_mock.h \
    harmony/mocks/sys_int_mock.cpp \
    harmony/mocks/sys_int_mock.h
harmony_mocks_libharmonymock_la_CXXFLAGS = \
   -I harmony/include $(GMOCK_INCLUDES) $(GTEST_INCLUDES) $(WARNING_CFLAGS)
harmony_mocks_libharmonymock_la_LIBADD = $(GMOCK_LIBS) $(GTEST_LIBS)

src_libflags_la_SOURCES = src/flags.c
src_libflags_la_CFLAGS = $(BUILD_FLAGS)

src_liblogger_la_SOURCES = src/logger.c
src_liblogger_la_CFLAGS = $(BUILD_FLAGS)

src_libmessagehandler_la_SOURCES = src/message_handler.c
src_libmessagehandler_la_CFLAGS = $(BUILD_FLAGS)

src_libstreamdecoder_la_SOURCES = src/stream_decoder.c
src_libstreamdecoder_la_CFLAGS = $(BUILD_FLAGS)

src_libtransceiver_la_SOURCES = src/transceiver.c
src_libtransceiver_la_CFLAGS = $(BUILD_FLAGS)

src_libusbtransport_la_SOURCES = src/usb_transport.c
src_libusbtransport_la_CFLAGS = $(BUILD_FLAGS)

include mocks/Makefile.mk

TESTS =

include tests/Makefile.mk

check_PROGRAMS = $(TESTS)

