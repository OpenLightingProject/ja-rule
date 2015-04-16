#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ja-rule], [0.0.1], [])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([config])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AC_LANG_PUSH([C++])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

LT_INIT
AC_PROG_LIBTOOL

# Check we have -std=c++11 support
AX_CXX_COMPILE_STDCXX_11(noext,mandatory)

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

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

# gmock
#####################################################
AC_ARG_WITH([gmock],
  [AS_HELP_STRING([--with-gmock=PATH], [The location of a built gmock)])],
  [], [with_gmock=../googlemock-read-only])

GMOCK_INCLUDES="-isystem $with_gmock/include"
GMOCK_LIBS="$with_gmock/lib/libgmock.la $with_gmock/lib/libgmock_main.la"
GTEST_INCLUDES="-isystem $with_gmock/gtest/include"
GTEST_LIBS="$with_gmock/gtest/lib/libgtest.la $with_gmock/gtest/lib/libgtest_main.la"

# save CPPFLAGS
old_cppflags=$CPPFLAGS
CPPFLAGS="$GMOCK_INCLUDES $GTEST_INCLUDES"

AC_CHECK_HEADER([gmock/gmock.h], [], [AC_MSG_ERROR([Missing gmock/gmock.h])])

# restore CPPFLAGS
CPPFLAGS=$old_cppflags

AC_SUBST([GMOCK_INCLUDES])
AC_SUBST([GMOCK_LIBS])
AC_SUBST([GTEST_INCLUDES])
AC_SUBST([GTEST_LIBS])

# Output
#####################################################
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo \
"-------------------------------------------------------
${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'
Compiler: '${CXX} ${CXXFLAGS} ${CPPFLAGS}'
Linker: '${LD} ${LDFLAGS} ${LIBS}'

Now type 'make @<:@<target>@:>@'
  where the optional <target> is:
    check        - run the tests
    doxygen-doc  - generate the html documentation
-------------------------------------------------------"
