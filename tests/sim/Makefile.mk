# Mocks
##################################################

noinst_LTLIBRARIES += tests/sim/libsim.la

tests_sim_libsim_la_SOURCES = tests/sim/Simulator.h \
                              tests/sim/Simulator.cpp \
                              tests/sim/PeripheralTimer.h \
                              tests/sim/PeripheralTimer.cpp \
                              tests/sim/PeripheralInputCapture.h \
                              tests/sim/PeripheralInputCapture.cpp \
                              tests/sim/PeripheralUART.h \
                              tests/sim/PeripheralUART.cpp \
                              tests/sim/InterruptController.h \
                              tests/sim/InterruptController.cpp
tests_sim_libsim_la_CXXFLAGS = $(BUILD_FLAGS) $(GMOCK_INCLUDES) \
                               $(GTEST_INCLUDES)  -I tests/harmony/mocks
tests_sim_libsim_la_LIBADD = $(GMOCK_LIBS) $(GTEST_LIBS)

