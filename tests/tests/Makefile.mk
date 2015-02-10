# TESTS
################################################
TESTING_CFLAGS = -I system_config -I ../src -I include \
                 $(WARNING_CFLAGS)

TESTING_CXXFLAGS = $(TESTING_CFLAGS) $(WARNING_CXXFLAGS) \
                   $(GMOCK_INCLUDES) $(GTEST_INCLUDES) -I ./mocks

TESTING_LIBS = $(GMOCK_LIBS) $(GTEST_LIBS)

TESTS += tests/flags_test \
         tests/logger_test \
         tests/message_handler_test \
         tests/stream_decoder_test \
         tests/usb_transport_test

tests_flags_test_SOURCES = tests/FlagsTest.cpp
tests_flags_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_flags_test_LDADD = $(TESTING_LIBS) \
                         src/libflags.la \
                         mocks/libtransportmock.la

tests_logger_test_SOURCES = tests/LoggerTest.cpp
tests_logger_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_logger_test_LDADD = $(TESTING_LIBS) \
                          src/liblogger.la \
                          mocks/libtransportmock.la

tests_stream_decoder_test_SOURCES = tests/StreamDecoderTest.cpp
tests_stream_decoder_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_stream_decoder_test_LDADD = $(TESTING_LIBS) \
                                  src/libstreamdecoder.la \
                                  mocks/libmessagehandlermock.la

tests_usb_transport_test_SOURCES = tests/USBTransportTest.cpp
tests_usb_transport_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_usb_transport_test_LDADD = $(TESTING_LIBS) \
                                 src/libusbtransport.la \
                                 mocks/libloggermock.la \
                                 mocks/libmatchers.la \
                                 mocks/libstreamdecodermock.la \
                                 mocks/libusbmock.la \
                                 src/libflags.la

tests_message_handler_test_SOURCES = tests/MessageHandlerTest.cpp
tests_message_handler_test_CXXFLAGS = $(TESTING_CXXFLAGS)
tests_message_handler_test_LDADD = $(GMOCK_LIBS) $(GTEST_LIBS) \
                                   mocks/libdmxmock.la \
                                   mocks/libflagsmock.la \
                                   mocks/libloggermock.la \
                                   mocks/libtransportmock.la \
                                   src/libmessagehandler.la
