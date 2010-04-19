AC_INIT([ngfd], [0.5])
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

CFLAGS="${CFLAGS} -Werror"

AC_ARG_ENABLE([debug],
	AS_HELP_STRING([--enable-debug],[Enable debug @<:@default=false@:>@]),
	[case "${enableval}" in
		yes) debug=true ; CFLAGS="-DENABLE_DEBUG" ;;
		no)  debug=false ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
	esac],
	[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AC_ARG_ENABLE([tonemapper],
	AS_HELP_STRING([--disable-tonemapper],[Disable tonemapper @<:@default=false@:>@]),
	[case "${enableval}" in
		yes) tonemapper=false ;;
		no)  tonemapper=true ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --disable-tonemapper]) ;;
	esac],
	[tonemapper=true])
AM_CONDITIONAL([TONEMAPPER], [test x$tonemapper = xtrue])

AC_ARG_ENABLE([vibrator],
	AS_HELP_STRING([--disable-vibrator],[Disable vibrator @<:@default=false@:>@]),
	[case "${enableval}" in
		yes) vibrator=false ;;
		no)  vibrator=true ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --disable-vibrator]) ;;
	esac],
	[vibrator=true])
AM_CONDITIONAL([VIBRATOR], [test x$vibrator = xtrue])

PKG_CHECK_MODULES(CHECK, check)
AC_SUBST(CHECK_LIBS)
AC_SUBST(CHECK_CFLAGS)

PKG_CHECK_MODULES(NGFD,
		  [glib-2.0
		   gobject-2.0
		   gthread-2.0
		   dbus-1
		   dbus-glib-1
		   libpulse
		   libpulse-mainloop-glib
		   profile
		   sndfile])
           
#### Gstreamer support (optional) ####

AC_ARG_ENABLE([gstreamer],
    AS_HELP_STRING([--disable-gstreamer],[Disable optional Gstreamer support]),
        [
            case "${enableval}" in
                yes) gstreamer=true ;;
                no) gstreamer=false ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --disable-gstreamer) ;;
            esac
        ],
        [gstreamer=true])

if test "x${gstreamer}" != xfalse ; then
    PKG_CHECK_MODULES(GST, [gstreamer-0.10 gio-2.0 ],
        [
            HAVE_GST=1
            AC_DEFINE([HAVE_GST], 1, [Have Gstreamer?])
        ],
        [
            HAVE_GST=0
            if test "x$gstreamer" = xtrue ; then
                AC_MSG_ERROR([*** Needed Gstreamer support not found])
            fi
        ])
else
    HAVE_GST=0
fi

AC_SUBST(GST_CFLAGS)
AC_SUBST(GST_LIBS)
AC_SUBST(HAVE_GST)
AM_CONDITIONAL([HAVE_GST], [test "x$HAVE_GST" = x1])

AC_SUBST(NGFD_LIBS)
AC_SUBST(NGFD_CFLAGS)

echo "
 == $PACKAGE_NAME $VERSION ==

    Compiler:               ${CC}
    CFLAGS:                 ${CFLAGS}

    Debug enabled:          ${debug}
    Vibrator enabled:       ${vibrator}
    Tone mapper enabled:    ${tonemapper}
    Gstreamer enabled:	     ${gstreamer}
"

AC_OUTPUT(Makefile src/Makefile tests/Makefile data/Makefile)
