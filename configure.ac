AC_INIT(src/dvbpsi.c)

AC_PREREQ(2.50)
AC_CONFIG_AUX_DIR(autotools)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE(libdvbpsi3, 0.1.4)
AM_CONFIG_HEADER(config.h)

#AC_CANONICAL_HOST
AC_PROG_CC
AC_STDC_HEADERS

AM_PROG_LIBTOOL


dnl default CFLAGS
CFLAGS="-Wall -DDVBPSI_DIST"

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
if test x$debug = xtrue
then
  CFLAGS="${CFLAGS} -DDEBUG -g"
fi


dnl --enable-release
AC_ARG_ENABLE(release,
[  --enable-release        Enable release mode (default disabled)],
[case "${enableval}" in
  yes) release=true ;;
  no)  release=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-release) ;;
esac],[release=false])
if test x$release = xtrue
then
  CFLAGS="${CFLAGS} -O6"
else
  CFLAGS="${CFLAGS} -O2"
fi

dnl Check for headers
AC_CHECK_HEADERS(stdint.h inttypes.h)

AC_OUTPUT([Makefile src/Makefile examples/Makefile misc/Makefile doc/Makefile debian/Makefile])

echo "
libdvbpsi configuration
-----------------------
libdvbpsi version     : ${VERSION}
debug                 : ${debug}
release               : ${release}
"

