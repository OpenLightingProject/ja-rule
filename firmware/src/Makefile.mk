noinst_LTLIBRARIES += firmware/src/libcoarsetimer.la \
                      firmware/src/libdimmermodel.la \
                      firmware/src/libflags.la \
                      firmware/src/libledmodel.la \
                      firmware/src/libmessagehandler.la \
                      firmware/src/libnetworkmodel.la \
                      firmware/src/libproxymodel.la \
                      firmware/src/librandom.la \
                      firmware/src/librdmbuffer.la \
                      firmware/src/librdmhandler.la \
                      firmware/src/librdmresponder.la \
                      firmware/src/librdmutil.la \
                      firmware/src/libreceivercounters.la \
                      firmware/src/libresponder.la \
                      firmware/src/libspirgb.la \
                      firmware/src/libstreamdecoder.la \
                      firmware/src/libtransceiver.la \
                      firmware/src/libusbtransport.la

firmware_src_libcoarsetimer_la_SOURCES = firmware/src/coarse_timer.c
firmware_src_libcoarsetimer_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libdimmermodel_la_SOURCES = firmware/src/dimmer_model.c
firmware_src_libdimmermodel_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libflags_la_SOURCES = firmware/src/flags.c
firmware_src_libflags_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libledmodel_la_SOURCES = firmware/src/led_model.c
firmware_src_libledmodel_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libmessagehandler_la_SOURCES = firmware/src/message_handler.c
firmware_src_libmessagehandler_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libnetworkmodel_la_SOURCES = firmware/src/network_model.c
firmware_src_libnetworkmodel_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libproxymodel_la_SOURCES = firmware/src/proxy_model.c
firmware_src_libproxymodel_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_librandom_la_SOURCES = firmware/src/random.c
firmware_src_librandom_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_librdmbuffer_la_SOURCES = firmware/src/rdm_buffer.c
firmware_src_librdmbuffer_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_librdmhandler_la_SOURCES = firmware/src/rdm_handler.c
firmware_src_librdmhandler_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_librdmresponder_la_SOURCES = firmware/src/rdm_responder.c
firmware_src_librdmresponder_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_librdmutil_la_SOURCES = firmware/src/rdm_util.c
firmware_src_librdmutil_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libreceivercounters_la_SOURCES = firmware/src/receiver_counters.c
firmware_src_libreceivercounters_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libresponder_la_SOURCES = firmware/src/responder.c
firmware_src_libresponder_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libspirgb_la_SOURCES = firmware/src/spi_rgb.c
firmware_src_libspirgb_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libstreamdecoder_la_SOURCES = firmware/src/stream_decoder.c
firmware_src_libstreamdecoder_la_CFLAGS = $(BUILD_FLAGS)

firmware_src_libtransceiver_la_SOURCES = firmware/src/transceiver.c
firmware_src_libtransceiver_la_CFLAGS = $(BUILD_FLAGS)
firmware_src_libtransceiver_la_LIBADD = firmware/src/librandom.la

firmware_src_libusbtransport_la_SOURCES = firmware/src/usb_transport.c
firmware_src_libusbtransport_la_CFLAGS = $(BUILD_FLAGS)
