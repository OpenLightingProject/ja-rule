# Mocks
##################################################

noinst_LTLIBRARIES += mocks/libappmock.la \
                      mocks/libcoarsetimermock.la \
                      mocks/libflagsmock.la \
                      mocks/libmatchers.la \
                      mocks/libmessagehandlermock.la \
                      mocks/librdmhandlermock.la \
                      mocks/libspirgbmock.la \
                      mocks/libstreamdecodermock.la \
                      mocks/libsyslogmock.la \
                      mocks/libtransceivermock.la \
                      mocks/libtransportmock.la \
                      mocks/libusbmock.la

MOCK_CXXFLAGS = $(BUILD_FLAGS) $(GMOCK_INCLUDES) $(GTEST_INCLUDES)
MOCK_LIBS = $(GMOCK_LIBS) $(GTEST_LIBS)

mocks_libappmock_la_SOURCES = mocks/AppMock.h mocks/AppMock.cpp
mocks_libappmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libappmock_la_LIBADD = $(MOCK_LIBS)

mocks_libcoarsetimermock_la_SOURCES = mocks/CoarseTimerMock,h \
                                      mocks/CoarseTimerMock.cpp
mocks_libcoarsetimermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libcoarsetimermock_la_LIBADD = $(MOCK_LIBS)

mocks_libflagsmock_la_SOURCES = mocks/FlagsMock.h mocks/FlagsMock.cpp
mocks_libflagsmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libflagsmock_la_LIBADD = $(MOCK_LIBS)

mocks_libmatchers_la_SOURCES = mocks/Matchers.h mocks/Matchers.cpp
mocks_libmatchers_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libmatchers_la_LIBADD = $(MOCK_LIBS)

mocks_libmessagehandlermock_la_SOURCES = mocks/MessageHandlerMock.h \
                                         mocks/MessageHandlerMock.cpp
mocks_libmessagehandlermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libmessagehandlermock_la_LIBADD = $(MOCK_LIBS)

mocks_librdmhandlermock_la_SOURCES = mocks/RDMHandlerMock.h \
                                     mocks/RDMHandlerMock.cpp
mocks_librdmhandlermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_librdmhandlermock_la_LIBADD = $(MOCK_LIBS)

mocks_libspirgbmock_la_SOURCES = mocks/SPIRGBMock.h mocks/SPIRGBMock.cpp
mocks_libspirgbmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libspirgbmock_la_LIBADD = $(MOCK_LIBS)

mocks_libstreamdecodermock_la_SOURCES = mocks/StreamDecoderMock.h \
                                        mocks/StreamDecoderMock.cpp
mocks_libstreamdecodermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libstreamdecodermock_la_LIBADD = $(MOCK_LIBS)

mocks_libsyslogmock_la_SOURCES = mocks/SysLogMock.h mocks/SysLogMock.cpp
mocks_libsyslogmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libsyslogmock_la_LIBADD = $(MOCK_LIBS)

mocks_libtransceivermock_la_SOURCES = mocks/TransceiverMock.h \
                                      mocks/TransceiverMock.cpp
mocks_libtransceivermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libtransceivermock_la_LIBADD = $(MOCK_LIBS)

mocks_libtransportmock_la_SOURCES = mocks/TransportMock.h \
                                    mocks/TransportMock.cpp
mocks_libtransportmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libtransportmock_la_LIBADD = $(MOCK_LIBS)

mocks_libusbmock_la_SOURCES = mocks/USBMock.h \
                              mocks/USBMock.cpp
mocks_libusbmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libusbmock_la_LIBADD = $(MOCK_LIBS)
