
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
                     src/libcoarsetimer.la \
                     src/libflags.la \
                     src/libmessagehandler.la \
                     src/libnetworkmodel.la \
                     src/librandom.la \
                     src/librdmbuffer.la \
                     src/librdmhandler.la \
                     src/librdmresponder.la \
                     src/librdmutil.la \
                     src/libresponder.la \
                     src/libspirgb.la \
                     src/libstreamdecoder.la \
                     src/libtransceiver.la \
                     src/libusbtransport.la

harmony_mocks_libharmonymock_la_SOURCES = \
    harmony/mocks/plib_ic_mock.cpp \
    harmony/mocks/plib_ic_mock.h \
    harmony/mocks/plib_ports_mock.cpp \
    harmony/mocks/plib_ports_mock.h \
    harmony/mocks/plib_spi_mock.h \
    harmony/mocks/plib_spi_mock.cpp \
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

src_libcoarsetimer_la_SOURCES = src/coarse_timer.c
src_libcoarsetimer_la_CFLAGS = $(BUILD_FLAGS)

src_libflags_la_SOURCES = src/flags.c
src_libflags_la_CFLAGS = $(BUILD_FLAGS)

src_libmessagehandler_la_SOURCES = src/message_handler.c
src_libmessagehandler_la_CFLAGS = $(BUILD_FLAGS)

src_libnetworkmodel_la_SOURCES = src/network_model.c
src_libnetworkmodel_la_CFLAGS = $(BUILD_FLAGS)

src_librandom_la_SOURCES = src/random.c
src_librandom_la_CFLAGS = $(BUILD_FLAGS)

src_librdmbuffer_la_SOURCES = src/rdm_buffer.c
src_librdmbuffer_la_CFLAGS = $(BUILD_FLAGS)

src_librdmhandler_la_SOURCES = src/rdm_handler.c
src_librdmhandler_la_CFLAGS = $(BUILD_FLAGS)

src_librdmresponder_la_SOURCES = src/rdm_responder.c
src_librdmresponder_la_CFLAGS = $(BUILD_FLAGS)

src_librdmutil_la_SOURCES = src/rdm_util.c
src_librdmutil_la_CFLAGS = $(BUILD_FLAGS)

src_libresponder_la_SOURCES = src/responder.c
src_libresponder_la_CFLAGS = $(BUILD_FLAGS)

src_libspirgb_la_SOURCES = src/spi_rgb.c
src_libspirgb_la_CFLAGS = $(BUILD_FLAGS)

src_libstreamdecoder_la_SOURCES = src/stream_decoder.c
src_libstreamdecoder_la_CFLAGS = $(BUILD_FLAGS)

src_libtransceiver_la_SOURCES = src/transceiver.c
src_libtransceiver_la_CFLAGS = $(BUILD_FLAGS)
src_libtransceiver_la_LIBADD = src/librandom.la

src_libusbtransport_la_SOURCES = src/usb_transport.c
src_libusbtransport_la_CFLAGS = $(BUILD_FLAGS)

include mocks/Makefile.mk

TESTS =

include tests/Makefile.mk

check_PROGRAMS = $(TESTS)

