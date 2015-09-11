
# As suggested by libtoolize
ACLOCAL_AMFLAGS = -I config

include aminclude.am

AM_CFLAGS = -Wall -Werror

WARNING_CFLAGS = -Wall -Wformat -W -Werror
WARNING_CXXFLAGS = -fvisibility-inlines-hidden

# Code Under Test
##################################################

BUILD_FLAGS = -I boardcfg \
              -I tests/system_config \
              -I tests/harmony/include \
              -I firmware/src \
              -I common \
              -I Bootloader/firmware/src \
              $(WARNING_CFLAGS)

noinst_PROGRAMS =

noinst_LTLIBRARIES =

TESTS =

###################

include Bootloader/firmware/src/Makefile.mk
include firmware/src/Makefile.mk
include tests/Makefile.mk
include tools/Makefile.mk

check_PROGRAMS = $(TESTS)

