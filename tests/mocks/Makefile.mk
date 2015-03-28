# Mocks
##################################################

noinst_LTLIBRARIES += mocks/libappmock.la \
                      mocks/libflagsmock.la \
                      mocks/libloggermock.la \
                      mocks/libmatchers.la \
                      mocks/libmessagehandlermock.la \
                      mocks/libstreamdecodermock.la \
                      mocks/libsyslogmock.la \
                      mocks/libtransceivermock.la \
                      mocks/libtransportmock.la \
                      mocks/libusbmock.la

MOCK_CXXFLAGS = $(BUILD_FLAGS) $(GMOCK_INCLUDES) $(GTEST_INCLUDES)
MOCK_LIBS = $(GMOCK_LIBS) $(GTEST_LIBS)

mocks_libappmock_la_SOURCES = mocks/AppMock.cpp
mocks_libappmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libappmock_la_LIBADD = $(MOCK_LIBS)

mocks_libflagsmock_la_SOURCES = mocks/FlagsMock.cpp
mocks_libflagsmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libflagsmock_la_LIBADD = $(MOCK_LIBS)

mocks_libloggermock_la_SOURCES = mocks/LoggerMock.cpp
mocks_libloggermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libloggermock_la_LIBADD = $(MOCK_LIBS)

mocks_libmatchers_la_SOURCES = mocks/Matchers.cpp
mocks_libmatchers_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libmatchers_la_LIBADD = $(MOCK_LIBS)

mocks_libmessagehandlermock_la_SOURCES = mocks/MessageHandlerMock.cpp
mocks_libmessagehandlermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libmessagehandlermock_la_LIBADD = $(MOCK_LIBS)

mocks_libstreamdecodermock_la_SOURCES = mocks/StreamDecoderMock.cpp
mocks_libstreamdecodermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libstreamdecodermock_la_LIBADD = $(MOCK_LIBS)

mocks_libsyslogmock_la_SOURCES = mocks/SysLogMock.cpp
mocks_libsyslogmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libsyslogmock_la_LIBADD = $(MOCK_LIBS)

mocks_libtransceivermock_la_SOURCES = mocks/TransceiverMock.cpp
mocks_libtransceivermock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libtransceivermock_la_LIBADD = $(MOCK_LIBS)

mocks_libtransportmock_la_SOURCES = mocks/TransportMock.cpp
mocks_libtransportmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libtransportmock_la_LIBADD = $(MOCK_LIBS)

mocks_libusbmock_la_SOURCES = mocks/USBMock.cpp
mocks_libusbmock_la_CXXFLAGS = $(MOCK_CXXFLAGS)
mocks_libusbmock_la_LIBADD = $(MOCK_LIBS)
