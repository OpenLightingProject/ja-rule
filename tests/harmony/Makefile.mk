noinst_LTLIBRARIES += tests/harmony/mocks/libharmonymock.la

tests_harmony_mocks_libharmonymock_la_SOURCES = \
    tests/harmony/mocks/plib_ic_mock.cpp \
    tests/harmony/mocks/plib_ic_mock.h \
    tests/harmony/mocks/plib_nvm_mock.cpp \
    tests/harmony/mocks/plib_nvm_mock.h \
    tests/harmony/mocks/plib_ports_mock.cpp \
    tests/harmony/mocks/plib_ports_mock.h \
    tests/harmony/mocks/plib_spi_mock.cpp \
    tests/harmony/mocks/plib_spi_mock.h \
    tests/harmony/mocks/plib_tmr_mock.cpp \
    tests/harmony/mocks/plib_tmr_mock.h \
    tests/harmony/mocks/plib_usart_mock.cpp \
    tests/harmony/mocks/plib_usart_mock.h \
    tests/harmony/mocks/sys_clk_mock.cpp \
    tests/harmony/mocks/sys_clk_mock.h \
    tests/harmony/mocks/sys_int_mock.cpp \
    tests/harmony/mocks/sys_int_mock.h \
    tests/harmony/mocks/usb_device_mock.cpp \
    tests/harmony/mocks/usb_device_mock.h

tests_harmony_mocks_libharmonymock_la_CXXFLAGS = \
   -I tests/harmony/include $(GMOCK_INCLUDES) $(GTEST_INCLUDES) \
   $(WARNING_CFLAGS)
tests_harmony_mocks_libharmonymock_la_LIBADD = $(GMOCK_LIBS) $(GTEST_LIBS)
