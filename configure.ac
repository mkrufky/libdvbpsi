AC_INIT(libdvbpsi, 1.0.0)

AC_PREREQ(2.50)
AC_CONFIG_AUX_DIR(.auto)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl AC_CANONICAL_HOST
AC_PROG_CC
AC_STDC_HEADERS
AC_C_INLINE

AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

dnl default CFLAGS
CFLAGS="${CFLAGS} -Wall -Werror --std=gnu99 -D_GNU_SOURCE"
CFLAGS="${CFLAGS} -Wpointer-arith -Wcast-align -Wcast-qual -Wstrict-prototypes -Wshadow -Waggregate-return -Wmissing-prototypes -Wnested-externs -Wsign-compare"

dnl check the operating system
case x"${target_os}" in
  xdarwin*)
    CFLAGS="${CFLAGS} -no-cpp-precomp"
    ;;
esac

dnl --enable-debug
AC_ARG_ENABLE(debug,
[  --enable-debug          Enable debug mode (default disabled)],
[case "${enableval}" in
  yes) debug=true ;;
  no)  debug=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
esac],[debug=false])
if test "$debug" = "true"
then
  CFLAGS="${CFLAGS} -g -ggdb3"
fi


dnl --enable-release
AC_ARG_ENABLE(release,
[  --enable-release        Enable release mode (default disabled)],
[case "${enableval}" in
  yes) release=true ;;
  no)  release=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-release) ;;
esac],[release=false])
if test "$release" = "true"
then
  CFLAGS="${CFLAGS} -O6"
else
  CFLAGS="${CFLAGS} -O2 -DDVBPSI_DIST"
fi

dnl Check for headers
AC_CHECK_HEADERS(stdbool.h stdint.h inttypes.h getopt.h strings.h sys/time.h)
dnl AC_CHECK_FUNCS([gettimeofday])

AC_CHECK_HEADERS(sys/socket.h, [ac_have_sys_socket_h=yes])
AM_CONDITIONAL(HAVE_SYS_SOCKET_H, test "${ac_have_sys_socket_h}" = "yes")

dnl Check for variadic macros
AC_CACHE_CHECK([for variadic cpp macros],
    [ac_cv_cpp_variadic_macros],
    [AC_TRY_COMPILE(
         [#include <stdio.h>
          #define a(b,c...) printf(b,##c)],
         [a("foo");a("%s","bar");a("%s%s","baz","quux");],
         ac_cv_cpp_variadic_macros=yes,
         ac_cv_cpp_variadic_macros=no)])
if test "${ac_cv_cpp_variadic_macros}" != "no"; then
    AC_DEFINE(HAVE_VARIADIC_MACROS, 1, Support for variadic macros)
fi

AC_OUTPUT([Makefile
           src/Makefile
           examples/Makefile
           examples/dvbinfo/Makefile
           misc/Makefile
           doc/Makefile
           wince/Makefile
           libdvbpsi.pc
           libdvbpsi.spec])

echo "
libdvbpsi configuration
-----------------------
libdvbpsi version     : ${VERSION}
debug                 : ${debug}
release               : ${release}
"

