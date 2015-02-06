#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([ja-rule], [0.0.1], [])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([config])
AM_INIT_AUTOMAKE([subdir-objects foreign])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.

LT_INIT
AC_PROG_LIBTOOL

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

DX_INIT_DOXYGEN($PACKAGE_NAME, Doxyfile)

# cppunit
#####################################################
AM_PATH_CPPUNIT([1.9.6], [],
                [AC_MSG_ERROR([Missing cppunit, please install >= 1.9.6])])

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
