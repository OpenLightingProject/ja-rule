# Mocks
##################################################

noinst_LTLIBRARIES += tests/sim/libsim.la

tests_sim_libsim_la_SOURCES = tests/sim/PeripheralTimer.h \
                              tests/sim/InterruptController.cpp \
                              tests/sim/InterruptController.h \
                              tests/sim/PeripheralInputCapture.cpp \
                              tests/sim/PeripheralInputCapture.h \
                              tests/sim/PeripheralSPI.cpp \
                              tests/sim/PeripheralSPI.h \
                              tests/sim/PeripheralTimer.cpp \
                              tests/sim/PeripheralUART.cpp \
                              tests/sim/PeripheralUART.h \
                              tests/sim/SignalGenerator.cpp \
                              tests/sim/SignalGenerator.h \
                              tests/sim/Simulator.cpp \
                              tests/sim/Simulator.h
tests_sim_libsim_la_CXXFLAGS = $(BUILD_FLAGS) $(GMOCK_INCLUDES) \
                               $(GTEST_INCLUDES)  -I tests/harmony/mocks
tests_sim_libsim_la_LIBADD = $(GMOCK_LIBS) $(GTEST_LIBS)

