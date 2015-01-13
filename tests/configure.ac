#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
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
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

AM_PATH_CPPUNIT([1.9.6], [],
                [AC_MSG_ERROR([Missing cppunit, please install >= 1.9.6])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
