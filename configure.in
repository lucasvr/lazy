dnl configure.in file for Lazy CD Player - 2000-Jan-20
AC_INIT(src/playcd.c)
AM_CONFIG_HEADER(include/config.h)
AM_INIT_AUTOMAKE(lazy,0.24e)

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_CHECK_LIB(nsl, main)
AC_CHECK_LIB(socket, main)

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/ioctl.h sys/time.h unistd.h linux/cdrom.h)

AC_PROG_GCC_TRADITIONAL
AC_TYPE_SIGNAL
AC_CHECK_FUNCS(mkdir socket strdup strerror strstr)

CFLAGS="-g"
FLAGS="$CFLAGS -Wall"
AC_OUTPUT([Makefile src/Makefile man/Makefile])
