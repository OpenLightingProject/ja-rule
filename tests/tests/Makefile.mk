# LIBS

noinst_LTLIBRARIES += tests/tests/libmodeltest.la \
                      tests/tests/libbootloaderhelper.la

tests_tests_libmodeltest_la_SOURCES = tests/tests/ModelTest.h \
                                      tests/tests/ModelTest.cpp
tests_tests_libmodeltest_la_CXXFLAGS= $(TESTING_CFLAGS) $(WARNING_CXXFLAGS) \
                                $(GMOCK_INCLUDES) $(GTEST_INCLUDES)

tests_tests_libbootloaderhelper_la_SOURCES = \
    tests/tests/BootloaderTestHelper.h \
    tests/tests/BootloaderTestHelper.cpp
tests_tests_libbootloaderhelper_la_CXXFLAGS= \
    $(TESTING_CFLAGS) $(WARNING_CXXFLAGS) \
    $(GMOCK_INCLUDES) $(GTEST_INCLUDES) \
    -I tests/mocks -I tests/harmony/mocks

# TESTS
################################################
TESTING_CFLAGS = $(BUILD_FLAGS) -I tests/include

TESTING_CXXFLAGS = $(TESTING_CFLAGS) $(WARNING_CXXFLAGS) \
                   $(GMOCK_INCLUDES) $(GTEST_INCLUDES) \
                   -I tests/mocks \
                   -I tests/harmony/mocks

TESTING_LIBS = $(GMOCK_LIBS) $(GTEST_LIBS)

TESTS += tests/tests/bootloader_test \
         tests/tests/bootloader_transfer_test \
         tests/tests/coarse_timer_test \
         tests/tests/dimmer_model_test \
         tests/tests/flags_test \
         tests/tests/led_model_test \
         tests/tests/message_handler_test \
         tests/tests/network_model_test \
         tests/tests/proxy_model_test \
         tests/tests/rdm_handler_test \
         tests/tests/rdm_responder_test \
         tests/tests/rdm_util_test \
         tests/tests/responder_test \
         tests/tests/spirgb_test \
         tests/tests/stream_decoder_test \
         tests/tests/transceiver_test \
         tests/tests/usb_transport_test \
         tests/tests/utils_test

tests_tests_bootloader_test_SOURCES = tests/tests/BootloaderTest.cpp
tests_tests_bootloader_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_bootloader_test_LDADD = $(TESTING_LIBS) \
                                    Bootloader/firmware/src/libbootloader.la \
                                    tests/harmony/mocks/libharmonymock.la \
                                    tests/mocks/libmatchers.la \
                                    tests/mocks/libbootloaderoptionsmock.la \
                                    tests/mocks/libflashmock.la \
                                    tests/mocks/liblaunchermock.la \
                                    tests/mocks/libresetmock.la \
                                    tests/tests/libbootloaderhelper.la

tests_tests_bootloader_transfer_test_SOURCES = \
    tests/tests/BootloaderTransferTest.cpp
tests_tests_bootloader_transfer_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_bootloader_transfer_test_LDADD = \
    $(TESTING_LIBS) \
    Bootloader/firmware/src/libbootloader.la \
    tests/harmony/mocks/libharmonymock.la \
    tests/mocks/libmatchers.la \
    tests/mocks/libbootloaderoptionsmock.la \
    tests/mocks/libflashmock.la \
    tests/mocks/liblaunchermock.la \
    tests/mocks/libresetmock.la \
    tests/tests/libbootloaderhelper.la

tests_tests_coarse_timer_test_SOURCES = tests/tests/CoarseTimerTest.cpp
tests_tests_coarse_timer_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_coarse_timer_test_LDADD = $(TESTING_LIBS) \
                                      firmware/src/libcoarsetimer.la \
                                      tests/harmony/mocks/libharmonymock.la

tests_tests_dimmer_model_test_SOURCES = tests/tests/DimmerModelTest.cpp
tests_tests_dimmer_model_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_tests_dimmer_model_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                      firmware/src/libdimmermodel.la \
                                      firmware/src/librdmresponder.la \
                                      firmware/src/libreceivercounters.la \
                                      firmware/src/libcoarsetimer.la \
                                      firmware/src/librdmbuffer.la \
                                      firmware/src/librdmutil.la \
                                      tests/tests/libmodeltest.la \
                                      tests/harmony/mocks/libharmonymock.la \
                                      tests/mocks/libmatchers.la

tests_tests_flags_test_SOURCES = tests/tests/FlagsTest.cpp
tests_tests_flags_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_flags_test_LDADD = $(TESTING_LIBS) \
                               firmware/src/libflags.la \
                               tests/mocks/libmatchers.la \
                               tests/mocks/libtransportmock.la

tests_tests_led_model_test_SOURCES = tests/tests/LEDModelTest.cpp
tests_tests_led_model_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_tests_led_model_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                   firmware/src/libledmodel.la \
                                   firmware/src/librdmresponder.la \
                                   firmware/src/libreceivercounters.la \
                                   firmware/src/libcoarsetimer.la \
                                   firmware/src/librdmbuffer.la \
                                   firmware/src/librandom.la \
                                   firmware/src/librdmutil.la \
                                   tests/tests/libmodeltest.la \
                                   tests/harmony/mocks/libharmonymock.la \
                                   tests/mocks/libmatchers.la

tests_tests_message_handler_test_SOURCES = tests/tests/MessageHandlerTest.cpp
tests_tests_message_handler_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_message_handler_test_LDADD = $(GMOCK_LIBS) $(GTEST_LIBS) \
                                         firmware/src/libmessagehandler.la \
                                         tests/mocks/libappmock.la \
                                         tests/mocks/libflagsmock.la \
                                         tests/mocks/libmatchers.la \
                                         tests/mocks/librdmhandlermock.la \
                                         tests/mocks/libsyslogmock.la \
                                         tests/mocks/libtransceivermock.la \
                                         tests/mocks/libtransportmock.la \
                                         tests/harmony/mocks/libharmonymock.la

tests_tests_network_model_test_SOURCES = tests/tests/NetworkModelTest.cpp
tests_tests_network_model_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_tests_network_model_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                       firmware/src/libnetworkmodel.la \
                                       firmware/src/librdmresponder.la \
                                       firmware/src/libreceivercounters.la \
                                       firmware/src/libcoarsetimer.la \
                                       firmware/src/librdmbuffer.la \
                                       firmware/src/librandom.la \
                                       firmware/src/librdmutil.la \
                                       tests/tests/libmodeltest.la \
                                       tests/harmony/mocks/libharmonymock.la \
                                       tests/mocks/libmatchers.la

tests_tests_proxy_model_test_SOURCES = tests/tests/ProxyModelTest.cpp
tests_tests_proxy_model_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_tests_proxy_model_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                     firmware/src/libproxymodel.la \
                                     firmware/src/librdmresponder.la \
                                     firmware/src/libreceivercounters.la \
                                     firmware/src/libcoarsetimer.la \
                                     firmware/src/librdmbuffer.la \
                                     firmware/src/librandom.la \
                                     firmware/src/librdmutil.la \
                                     tests/tests/libmodeltest.la \
                                     tests/harmony/mocks/libharmonymock.la \
                                     tests/mocks/libmatchers.la

tests_tests_rdm_handler_test_SOURCES = tests/tests/RDMHandlerTest.cpp
tests_tests_rdm_handler_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_tests_rdm_handler_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                     tests/mocks/libmatchers.la \
                                     firmware/src/librdmhandler.la \
                                     firmware/src/librdmresponder.la \
                                     firmware/src/libreceivercounters.la \
                                     firmware/src/libcoarsetimer.la \
                                     firmware/src/librdmbuffer.la \
                                     firmware/src/librdmutil.la \
                                     tests/harmony/mocks/libharmonymock.la

tests_tests_rdm_responder_test_SOURCES = tests/tests/RDMResponderTest.cpp
tests_tests_rdm_responder_test_CXXFLAGS = $(TESTING_CXXFLAGS) $(OLA_CFLAGS)
tests_tests_rdm_responder_test_LDADD = $(TESTING_LIBS) $(OLA_LIBS) \
                                       firmware/src/librdmresponder.la \
                                       firmware/src/libreceivercounters.la \
                                       firmware/src/librdmbuffer.la \
                                       firmware/src/libcoarsetimer.la \
                                       firmware/src/librdmutil.la \
                                       tests/harmony/mocks/libharmonymock.la \
                                       tests/mocks/libmatchers.la \
                                       tests/mocks/libmessagehandlermock.la

tests_tests_rdm_util_test_SOURCES = tests/tests/RDMUtilTest.cpp
tests_tests_rdm_util_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_rdm_util_test_LDADD = $(TESTING_LIBS) \
                                  firmware/src/librdmutil.la \
                                  tests/mocks/libmatchers.la

tests_tests_responder_test_SOURCES = tests/tests/ResponderTest.cpp
tests_tests_responder_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_responder_test_LDADD = $(TESTING_LIBS) \
                                   firmware/src/librdmutil.la \
                                   firmware/src/libreceivercounters.la \
                                   firmware/src/libresponder.la \
                                   firmware/src/librdmutil.la \
                                   tests/mocks/libmatchers.la \
                                   tests/mocks/librdmhandlermock.la \
                                   tests/mocks/libspirgbmock.la \
                                   tests/mocks/libsyslogmock.la

tests_tests_spirgb_test_SOURCES = tests/tests/SPIRGBTest.cpp
tests_tests_spirgb_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_spirgb_test_LDADD = $(TESTING_LIBS) \
                                firmware/src/libspirgb.la \
                                tests/harmony/mocks/libharmonymock.la \
                                tests/mocks/libmatchers.la

tests_tests_stream_decoder_test_SOURCES = tests/tests/StreamDecoderTest.cpp
tests_tests_stream_decoder_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_stream_decoder_test_LDADD = $(TESTING_LIBS) \
                                        firmware/src/libstreamdecoder.la \
                                        tests/mocks/libmessagehandlermock.la

tests_tests_usb_transport_test_SOURCES = tests/tests/USBTransportTest.cpp
tests_tests_usb_transport_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_usb_transport_test_LDADD = $(TESTING_LIBS) \
                                       firmware/src/libusbtransport.la \
                                       tests/harmony/mocks/libharmonymock.la \
                                       tests/mocks/libbootloaderoptionsmock.la \
                                       tests/mocks/libmatchers.la \
                                       tests/mocks/libresetmock.la \
                                       tests/mocks/libstreamdecodermock.la \
                                       firmware/src/libflags.la

tests_tests_transceiver_test_SOURCES = tests/tests/TransceiverTest.cpp
tests_tests_transceiver_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_transceiver_test_LDADD = $(GMOCK_LIBS) $(GTEST_LIBS) \
                                     firmware/src/libtransceiver.la \
                                     tests/harmony/mocks/libharmonymock.la \
                                     tests/mocks/libcoarsetimermock.la \
                                     tests/mocks/libsyslogmock.la

tests_tests_utils_test_SOURCES = tests/tests/UtilsTest.cpp
tests_tests_utils_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_tests_utils_test_LDADD = $(GMOCK_LIBS) $(GTEST_LIBS) \
                               tests/mocks/libmatchers.la
