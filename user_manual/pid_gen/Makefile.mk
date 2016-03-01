# LIBS

noinst_PROGRAMS += user_manual/pid_gen/pid_gen

user_manual_pid_gen_pid_gen_SOURCES = user_manual/pid_gen/pid_gen.cpp
user_manual_pid_gen_pid_gen_CXXFLAGS = $(BUILD_FLAGS)  -I tests/include \
                                       $(GMOCK_INCLUDES) $(GTEST_INCLUDES) \
                                       $(OLA_CFLAGS)
user_manual_pid_gen_pid_gen_LDADD = $(OLA_LIBS) \
                                    firmware/src/libdimmermodel.la \
                                    firmware/src/libledmodel.la \
                                    firmware/src/libmovinglightmodel.la \
                                    firmware/src/libnetworkmodel.la \
                                    firmware/src/libproxymodel.la \
                                    firmware/src/librandom.la \
                                    firmware/src/librdmbuffer.la \
                                    firmware/src/librdmresponder.la \
                                    firmware/src/librdmutil.la \
                                    firmware/src/libreceivercounters.la \
                                    firmware/src/libsensormodel.la \
                                    firmware/src/libcoarsetimer.la \
                                    tests/harmony/mocks/libharmonymock.la \
                                    $(GMOCK_LIBS) $(GTEST_LIBS)
