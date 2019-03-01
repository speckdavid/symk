# $Id$
#
# Process this file with autoconf to produce a configure script.
#

AC_PREREQ(2.59)
AC_INIT([termview], [0.1], [asminer@iastate.edu], termview, [http://meddly.sourceforge.net])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])
m4_include([m4/ax_with_curses.m4])
AC_PREFIX_DEFAULT([$PWD])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AX_WITH_CURSES
if test "x$ax_cv_curses" != xyes; then
  AC_MSG_ERROR([requires an X/Open-compatible Curses library])
fi


# Wrap up

AC_CONFIG_FILES([Makefile
                  src/Makefile])

AC_OUTPUT
