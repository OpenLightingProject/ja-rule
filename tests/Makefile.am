
# As suggested by libtoolize
ACLOCAL_AMFLAGS = -I config

include aminclude.am

AM_CFLAGS = -Wall -Werror

WARNING_CFLAGS = -Wall -Wformat -W -Werror
WARNING_CXXFLAGS = -fvisibility-inlines-hidden

# Code Under Test
##################################################

BUILD_FLAGS = -I system_config -I ../src $(WARNING_CFLAGS)

noinst_LTLIBRARIES = src/libflags.la \
                     src/liblogger.la \
                     src/libmessagehandler.la \
                     src/libstreamdecoder.la \
                     src/libusbtransport.la

src_libflags_la_SOURCES = src/flags.c
src_libflags_la_CFLAGS = $(BUILD_FLAGS)

src_liblogger_la_SOURCES = src/logger.c
src_liblogger_la_CFLAGS = $(BUILD_FLAGS)

src_libmessagehandler_la_SOURCES = src/message_handler.c
src_libmessagehandler_la_CFLAGS = $(BUILD_FLAGS)

src_libstreamdecoder_la_SOURCES = src/stream_decoder.c
src_libstreamdecoder_la_CFLAGS = $(BUILD_FLAGS)

src_libusbtransport_la_SOURCES = src/usb_transport.c
src_libusbtransport_la_CFLAGS = $(BUILD_FLAGS)

include mocks/Makefile.mk

TESTS =

include tests/Makefile.mk

check_PROGRAMS = $(TESTS)

