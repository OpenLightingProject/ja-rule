# TESTS
################################################
TESTING_CFLAGS = $(BUILD_FLAGS) -I include

TESTING_CXXFLAGS = $(TESTING_CFLAGS) $(WARNING_CXXFLAGS) \
                   $(GMOCK_INCLUDES) $(GTEST_INCLUDES) -I ./mocks \
                   -I ./harmony/mocks

TESTING_LIBS = $(GMOCK_LIBS) $(GTEST_LIBS)

TESTS += tests/coarse_timer_test \
         tests/flags_test \
         tests/message_handler_test \
         tests/rdm_responder_test \
         tests/responder_test \
         tests/spirgb_test \
         tests/stream_decoder_test \
         tests/transceiver_test \
         tests/usb_transport_test

tests_coarse_timer_test_SOURCES = tests/CoarseTimerTest.cpp
tests_coarse_timer_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_coarse_timer_test_LDADD = $(TESTING_LIBS) \
                         src/libcoarsetimer.la \
                         harmony/mocks/libharmonymock.la

tests_flags_test_SOURCES = tests/FlagsTest.cpp
tests_flags_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_flags_test_LDADD = $(TESTING_LIBS) \
                         src/libflags.la \
                         mocks/libmatchers.la \
                         mocks/libtransportmock.la

tests_message_handler_test_SOURCES = tests/MessageHandlerTest.cpp
tests_message_handler_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_message_handler_test_LDADD = $(GMOCK_LIBS) $(GTEST_LIBS) \
                                   src/libmessagehandler.la \
                                   mocks/libappmock.la \
                                   mocks/libflagsmock.la \
                                   mocks/libmatchers.la \
                                   mocks/librdmrespondermock.la \
                                   mocks/libsyslogmock.la \
                                   mocks/libtransceivermock.la \
                                   mocks/libtransportmock.la

tests_rdm_responder_test_SOURCES = tests/RDMResponderTest.cpp
tests_rdm_responder_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_rdm_responder_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                  src/librdmresponder.la \
                                  harmony/mocks/libharmonymock.la \
                                  mocks/libcoarsetimermock.la \
                                  mocks/libmatchers.la \
                                  mocks/libmessagehandlermock.la

tests_responder_test_SOURCES = tests/ResponderTest.cpp
tests_responder_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_responder_test_LDADD = $(TESTING_LIBS) \
                             src/libresponder.la \
                             mocks/libmatchers.la \
                             mocks/librdmrespondermock.la \
                             mocks/libspirgbmock.la \
                             mocks/libsyslogmock.la

tests_spirgb_test_SOURCES = tests/SPIRGBTest.cpp
tests_spirgb_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_spirgb_test_LDADD = $(TESTING_LIBS) \
                          src/libspirgb.la \
                          harmony/mocks/libharmonymock.la \
                          mocks/libmatchers.la

tests_stream_decoder_test_SOURCES = tests/StreamDecoderTest.cpp
tests_stream_decoder_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_stream_decoder_test_LDADD = $(TESTING_LIBS) \
                                  src/libstreamdecoder.la \
                                  mocks/libmessagehandlermock.la

tests_usb_transport_test_SOURCES = tests/USBTransportTest.cpp
tests_usb_transport_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_usb_transport_test_LDADD = $(TESTING_LIBS) \
                                 src/libusbtransport.la \
                                 mocks/libmatchers.la \
                                 mocks/libstreamdecodermock.la \
                                 mocks/libusbmock.la \
                                 src/libflags.la

tests_transceiver_test_SOURCES = tests/TransceiverTest.cpp
tests_transceiver_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_transceiver_test_LDADD = $(GMOCK_LIBS) $(GTEST_LIBS) \
                               src/libtransceiver.la \
                               harmony/mocks/libharmonymock.la \
                               mocks/libcoarsetimermock.la \
                               mocks/libsyslogmock.la

