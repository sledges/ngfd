AC_INIT([ngfd], [0.1])
AC_CANONICAL_SYSTEM
AC_CONFIG_MACRO_DIR([m4])

AM_CONFIG_HEADER(config.h)
AC_CONFIG_SRCDIR([src/main.c])

AM_INIT_AUTOMAKE

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AM_PROG_LIBTOOL

AC_HEADER_STDC

AC_PATH_X
AC_PATH_XTRA
AC_SUBST(X_CFLAGS)

ngfconfdir=${sysconfdir}/ngf
AC_SUBST(ngfconfdir)

PKG_CHECK_MODULES(NGFD,
		  [glib-2.0
		   dbus-1
		   dbus-glib-1
		   libpulse
		   libpulse-mainloop-glib
		   sndfile])

AC_SUBST(NGFD_LIBS)
AC_SUBST(NGFD_CFLAGS)

AC_OUTPUT(Makefile src/Makefile tests/Makefile data/Makefile)
