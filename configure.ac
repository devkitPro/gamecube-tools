#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([gamecube-tools],[1.0.6],[https://github.com/devkitPro/gamecube-tools/issues])
AC_CONFIG_SRCDIR([gdtool/gdtool.cpp])

AM_INIT_AUTOMAKE([subdir-objects])
LT_INIT([disable-shared])

AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

case "$host" in
   *-*-mingw*)
      LIBS="$LIBS -lws2_32"
      ;;

esac

case "$host" in
   x86_64-apple-darwin* | aarch64-apple-darwin* )
      STDCPP_LIBS="-lc++"
      ;;

   *)
      STDCPP_LIBS="-lstdc++"
      ;;

esac

AC_CHECK_HEADER([FreeImage.h])

AC_MSG_CHECKING([for libfreeimage])
save_LIBS="$LIBS"
LIBS="-lfreeimage ${LIBS} ${STDCPP_LIBS} -lm"
AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([[#include <FreeImage.h>]],
        [[FreeImage_DeInitialise()]])],
        [freeimage_result=yes],
        [freeimage_result=no])
AC_MSG_RESULT([$freeimage_result])
LIBS="$save_LIBS"
if test "x$freeimage_result" = "xyes"; then
        FREEIMAGE_LIBS="-lfreeimage ${STDCPP_LIBS} -lm"
else
         AC_MSG_ERROR(['libfreeimage' not found])
fi
AC_SUBST([FREEIMAGE_LIBS], [${FREEIMAGE_LIBS}])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
