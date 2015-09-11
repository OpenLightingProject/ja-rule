# Mocks
##################################################

noinst_LTLIBRARIES += tests/mocks/libappmock.la \
                      tests/mocks/libbootloaderoptionsmock.la \
                      tests/mocks/libcoarsetimermock.la \
                      tests/mocks/libflagsmock.la \
                      tests/mocks/libflashmock.la \
                      tests/mocks/liblaunchermock.la \
                      tests/mocks/libmatchers.la \
                      tests/mocks/libmessagehandlermock.la \
                      tests/mocks/librdmhandlermock.la \
                      tests/mocks/libresetmock.la \
                      tests/mocks/libspirgbmock.la \
                      tests/mocks/libstreamdecodermock.la \
                      tests/mocks/libsyslogmock.la \
                      tests/mocks/libtransceivermock.la \
                      tests/mocks/libtransportmock.la

MOCK_CXXFLAGS = $(BUILD_FLAGS) $(GMOCK_INCLUDES) $(GTEST_INCLUDES)
MOCK_LIBS = $(GMOCK_LIBS) $(GTEST_LIBS)

tests_mocks_libappmock_la_SOURCES = tests/mocks/AppMock.h \
                                    tests/mocks/AppMock.cpp
tests_mocks_libappmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libappmock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libbootloaderoptionsmock_la_SOURCES = \
    tests/mocks/BootloaderOptionsMock.h \
    tests/mocks/BootloaderOptionsMock.cpp
tests_mocks_libbootloaderoptionsmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libbootloaderoptionsmock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libcoarsetimermock_la_SOURCES = tests/mocks/CoarseTimerMock,h \
                                            tests/mocks/CoarseTimerMock.cpp
tests_mocks_libcoarsetimermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libcoarsetimermock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libflagsmock_la_SOURCES = tests/mocks/FlagsMock.h \
                                      tests/mocks/FlagsMock.cpp
tests_mocks_libflagsmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libflagsmock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libflashmock_la_SOURCES = tests/mocks/FlashMock.h \
                                      tests/mocks/FlashMock.cpp
tests_mocks_libflashmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libflashmock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_liblaunchermock_la_SOURCES = tests/mocks/LauncherMock.h \
                                         tests/mocks/LauncherMock.cpp
tests_mocks_liblaunchermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_liblaunchermock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libmatchers_la_SOURCES = tests/mocks/Matchers.h \
                                     tests/mocks/Matchers.cpp
tests_mocks_libmatchers_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libmatchers_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libmessagehandlermock_la_SOURCES = \
    tests/mocks/MessageHandlerMock.h \
    tests/mocks/MessageHandlerMock.cpp
tests_mocks_libmessagehandlermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libmessagehandlermock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_librdmhandlermock_la_SOURCES = tests/mocks/RDMHandlerMock.h \
                                           tests/mocks/RDMHandlerMock.cpp
tests_mocks_librdmhandlermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_librdmhandlermock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libresetmock_la_SOURCES = tests/mocks/ResetMock.h \
                                      tests/mocks/ResetMock.cpp
tests_mocks_libresetmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libresetmock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libspirgbmock_la_SOURCES = tests/mocks/SPIRGBMock.h \
                                       tests/mocks/SPIRGBMock.cpp
tests_mocks_libspirgbmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libspirgbmock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libstreamdecodermock_la_SOURCES = tests/mocks/StreamDecoderMock.h \
                                              tests/mocks/StreamDecoderMock.cpp
tests_mocks_libstreamdecodermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libstreamdecodermock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libsyslogmock_la_SOURCES = tests/mocks/SysLogMock.h \
                                       tests/mocks/SysLogMock.cpp
tests_mocks_libsyslogmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libsyslogmock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libtransceivermock_la_SOURCES = tests/mocks/TransceiverMock.h \
                                            tests/mocks/TransceiverMock.cpp
tests_mocks_libtransceivermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libtransceivermock_la_LIBADD = $(MOCK_LIBS)

tests_mocks_libtransportmock_la_SOURCES = tests/mocks/TransportMock.h \
                                          tests/mocks/TransportMock.cpp
tests_mocks_libtransportmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
tests_mocks_libtransportmock_la_LIBADD = $(MOCK_LIBS)
