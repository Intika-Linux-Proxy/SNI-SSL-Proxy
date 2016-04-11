#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT([sniproxy], [0.1.0], [https://github.com/XiaoxiaoPu/sniproxy/issues], [sniproxy], [https://github.com/XiaoxiaoPu/sniproxy])
AC_CONFIG_SRCDIR([sniproxy])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_AUX_DIR(.)
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for programs.
: ${CFLAGS=""}
AC_PROG_CC_C99
AC_PROG_INSTALL

# Custom options
AC_ARG_ENABLE(
    [static],
    [AS_HELP_STRING([--enable-static], [build with static linking])],
    [LDFLAGS="$LDFLAGS -static"])
AM_CONDITIONAL(STATIC, test x"$static" = x"true")
AC_ARG_ENABLE(
    [debug],
    [AS_HELP_STRING([--enable-debug], [build with additional debugging code])],
    [CFLAGS="$CFLAGS -g -DDEBUG -O0"])
AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

# Checks for libraries.
LIB_MILL="-lmill"
AC_CHECK_LIB([mill], [now], [AC_SUBST(LIB_MILL)])

# Checks for header files.
AC_HEADER_ASSERT
AC_CHECK_HEADERS([arpa/inet.h stdint.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_PID_T

# Checks for library functions.
AC_CHECK_FUNCS([bzero sigaction strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
