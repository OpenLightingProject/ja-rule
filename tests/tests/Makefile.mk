# LIBS

noinst_LTLIBRARIES += tests/libmodeltest.la

tests_libmodeltest_la_SOURCES = tests/ModelTest.h \
                                tests/ModelTest.cpp
tests_libmodeltest_la_CXXFLAGS= $(TESTING_CFLAGS) $(WARNING_CXXFLAGS) \
                                $(GMOCK_INCLUDES) $(GTEST_INCLUDES)

# TESTS
################################################
TESTING_CFLAGS = $(BUILD_FLAGS) -I include

TESTING_CXXFLAGS = $(TESTING_CFLAGS) $(WARNING_CXXFLAGS) \
                   $(GMOCK_INCLUDES) $(GTEST_INCLUDES) -I ./mocks \
                   -I ./harmony/mocks

TESTING_LIBS = $(GMOCK_LIBS) $(GTEST_LIBS)

TESTS += tests/coarse_timer_test \
         tests/dimmer_model_test \
         tests/flags_test \
         tests/message_handler_test \
         tests/network_model_test \
         tests/proxy_model_test \
         tests/rdm_handler_test \
         tests/rdm_responder_test \
         tests/rdm_util_test \
         tests/responder_test \
         tests/simple_model_test \
         tests/spirgb_test \
         tests/stream_decoder_test \
         tests/transceiver_test \
         tests/usb_transport_test \
         tests/utils_test

tests_coarse_timer_test_SOURCES = tests/CoarseTimerTest.cpp
tests_coarse_timer_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_coarse_timer_test_LDADD = $(TESTING_LIBS) \
                         src/libcoarsetimer.la \
                         harmony/mocks/libharmonymock.la

tests_dimmer_model_test_SOURCES = tests/DimmerModelTest.cpp
tests_dimmer_model_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_dimmer_model_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                 src/libdimmermodel.la \
                                 src/librdmresponder.la \
                                 src/libcoarsetimer.la \
                                 src/librdmbuffer.la \
                                 src/librdmutil.la \
                                 tests/libmodeltest.la \
                                 harmony/mocks/libharmonymock.la \
                                 mocks/libmatchers.la

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
                                   mocks/librdmhandlermock.la \
                                   mocks/libsyslogmock.la \
                                   mocks/libtransceivermock.la \
                                   mocks/libtransportmock.la

tests_network_model_test_SOURCES = tests/NetworkModelTest.cpp
tests_network_model_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_network_model_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                 src/libnetworkmodel.la \
                                 src/librdmresponder.la \
                                 src/libcoarsetimer.la \
                                 src/librdmbuffer.la \
                                 src/librandom.la \
                                 src/librdmutil.la \
                                 tests/libmodeltest.la \
                                 harmony/mocks/libharmonymock.la \
                                 mocks/libmatchers.la

tests_proxy_model_test_SOURCES = tests/ProxyModelTest.cpp
tests_proxy_model_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_proxy_model_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                src/libproxymodel.la \
                                src/librdmresponder.la \
                                src/libcoarsetimer.la \
                                src/librdmbuffer.la \
                                src/librandom.la \
                                src/librdmutil.la \
                                tests/libmodeltest.la \
                                harmony/mocks/libharmonymock.la \
                                mocks/libmatchers.la

tests_rdm_handler_test_SOURCES = tests/RDMHandlerTest.cpp
tests_rdm_handler_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_rdm_handler_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                               mocks/libmatchers.la \
                               src/librdmhandler.la \
                               src/librdmresponder.la \
                               src/libcoarsetimer.la \
                               src/librdmbuffer.la \
                               src/librdmutil.la \
                               harmony/mocks/libharmonymock.la

tests_rdm_responder_test_SOURCES = tests/RDMResponderTest.cpp
tests_rdm_responder_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_rdm_responder_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                  src/librdmresponder.la \
                                  src/librdmbuffer.la \
                                  src/libcoarsetimer.la \
                                  src/librdmutil.la \
                                  harmony/mocks/libharmonymock.la \
                                  mocks/libmatchers.la \
                                  mocks/libmessagehandlermock.la

tests_rdm_util_test_SOURCES = tests/RDMUtilTest.cpp
tests_rdm_util_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_rdm_util_test_LDADD = $(TESTING_LIBS) \
                            src/librdmutil.la \
                            mocks/libmatchers.la

tests_responder_test_SOURCES = tests/ResponderTest.cpp
tests_responder_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_responder_test_LDADD = $(TESTING_LIBS) \
                             src/libresponder.la \
                             src/librdmutil.la \
                             mocks/libmatchers.la \
                             mocks/librdmhandlermock.la \
                             mocks/libspirgbmock.la \
                             mocks/libsyslogmock.la

tests_simple_model_test_SOURCES = tests/SimpleModelTest.cpp
tests_simple_model_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_simple_model_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                src/libsimplemodel.la \
                                src/librdmresponder.la \
                                src/libcoarsetimer.la \
                                src/librdmbuffer.la \
                                src/librandom.la \
                                src/librdmutil.la \
                                tests/libmodeltest.la \
                                harmony/mocks/libharmonymock.la \
                                mocks/libmatchers.la

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

tests_utils_test_SOURCES = tests/UtilsTest.cpp
tests_utils_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_utils_test_LDADD = $(GMOCK_LIBS) $(GTEST_LIBS) \
                         mocks/libmatchers.la
