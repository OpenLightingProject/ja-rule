
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

if BUILD_UNIT_TESTS
include Bootloader/firmware/src/Makefile.mk
include firmware/src/Makefile.mk
include tests/Makefile.mk
endif

include tools/Makefile.mk
include user_manual/Makefile.mk

check_PROGRAMS = $(TESTS)

doc-gen: user_manual/pid_gen/pid_gen
	./user_manual/pid_gen/pid_gen --output-dir user_manual/gen

