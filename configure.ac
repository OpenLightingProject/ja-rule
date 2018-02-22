#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ja-rule], [0.0.1], [])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([config])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AC_LANG_PUSH(C)
AC_PROG_CC
AC_LANG_POP(C)

# Checks for libraries.

LT_INIT
AC_PROG_LIBTOOL

AC_LANG_PUSH(C)
# Switch the C compiler into C99 mode
AC_PROG_CC_C99
AC_LANG_POP(C)

# Allow us to just build the tools without the unit tests
AC_ARG_ENABLE(
  [unit-tests],
  [AS_HELP_STRING([--disable-unit-tests],
  [Just build the tools without the unit tests])])
AM_CONDITIONAL(BUILD_UNIT_TESTS, test "x$enable_unit_tests" != xno)

# Check we have -std=c++11 support if required
AM_COND_IF([BUILD_UNIT_TESTS],
           [AX_CXX_COMPILE_STDCXX_11(noext,mandatory)])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

# Enable gcov to produce coverage data.
AC_ARG_ENABLE(
  [gcov],
  [AS_HELP_STRING([--enable-gcov], [Turn on code coverage analysis tools])])
AS_IF([test "x$enable_gcov" = xyes],
      [CFLAGS="$CFLAGS -fprofile-arcs -ftest-coverage"
       CXXFLAGS="$CXXFLAGS -fprofile-arcs -ftest-coverage"
       LIBS="$LIBS -lgcov"])

# Optionally set the Doxygen version to "Latest Git" for website latest
# version.
AC_ARG_ENABLE(
  [doxygen-version],
  [AS_HELP_STRING([--disable-doxygen-version],
  [Substitute the Doxygen version with latest, for the website])])

# Doxygen
#####################################################
DX_DOT_FEATURE(ON)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)

DX_ARG_ABLE(verbose, [Verbose doxygen output],
            [], [], [],
            [DX_ENV_APPEND(QUIET, NO)],
            [DX_ENV_APPEND(QUIET, YES)])
doxygen_project_number=$PACKAGE_VERSION
AS_IF([test "x$enable_doxygen_version" = xno],
      [doxygen_project_number="Latest Git"])
DX_ENV_APPEND(PROJECT_NUMBER, $doxygen_project_number)

DX_INIT_DOXYGEN($PACKAGE_NAME, Doxyfile, [], 1.8.0)

# pthread
ACX_PTHREAD([
             LIBS="$PTHREAD_LIBS $LIBS"
             CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
             CXXFLAGS="$CXXFLAGS $PTHREAD_CXXFLAGS"
             CC="$PTHREAD_CC"
             CXX="$PTHREAD_CXX"],
            [AC_MSG_ERROR([Missing pthread, please install it])])

# gmock
#####################################################
AC_ARG_WITH([gmock],
  [AS_HELP_STRING([--with-gmock=PATH], [The location of a built gmock)])],
  [], [with_gmock=./gtest-read-only])

# This is a mess, see
# http://stackoverflow.com/questions/27091835/undefined-reference-to-pthread-key-create-argument-order-does-not-fix-issue
GMOCK_INCLUDES="-isystem $with_gmock/googlemock/include"
GMOCK_LIBS="$PTHREAD_CFLAGS $with_gmock/googlemock/lib/.libs/libgmock_main.a $with_gmock/googlemock/lib/.libs/libgmock.a"
GTEST_INCLUDES="-isystem $with_gmock/googletest/include"
# Ideally do some clever detection of which flavour of gmock people have, for now they'll just need to upgrade...
# GTEST_LIBS="$PTHREAD_CFLAGS $with_gmock/googletest/lib/.libs/libgtest.a $with_gmock/googletest/lib/.libs/libgtest_main.a"
GTEST_LIBS="$PTHREAD_CFLAGS $with_gmock/googletest/googletest/lib/.libs/libgtest.a $with_gmock/googletest/googlemock/lib/.libs/libgmock_main.a"

# save CPPFLAGS
old_cppflags=$CPPFLAGS
CPPFLAGS="$GMOCK_INCLUDES $GTEST_INCLUDES"

AM_COND_IF([BUILD_UNIT_TESTS],
           [AC_CHECK_HEADER([gmock/gmock.h],
                            [],
                            [AC_MSG_ERROR([Missing gmock/gmock.h])])])

# restore CPPFLAGS
CPPFLAGS=$old_cppflags

AC_SUBST([GMOCK_INCLUDES])
AC_SUBST([GMOCK_LIBS])
AC_SUBST([GTEST_INCLUDES])
AC_SUBST([GTEST_LIBS])

# pkg-config
PKG_PROG_PKG_CONFIG
AS_IF([test -z "$PKG_CONFIG"],
      [AC_MSG_ERROR([Missing pkg-config, please install it])])

# OLA
AC_ARG_WITH(
  [ola],
  [AS_HELP_STRING([--without-ola],
                  [Skip OLA checks, unittests will not work correctly])])
AS_IF([test "x$enable_unit_tests" != xno && test "x$with_ola" != xno],
      [PKG_CHECK_MODULES(OLA,
                         [libola],
                         [],
                         [AC_MSG_ERROR([Missing OLA, please install])])])

# Output
#####################################################
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo \
"-------------------------------------------------------
${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'
C++ Compiler: '${CXX} ${CXXFLAGS} ${CPPFLAGS}'
C Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'
Linker: '${LD} ${LDFLAGS} ${LIBS}'

Unit Tests: ${enable_unit_tests}

Now type 'make @<:@<target>@:>@'
  where the optional <target> is:
    check        - run the tests
    doxygen-doc  - generate the html documentation
-------------------------------------------------------"
