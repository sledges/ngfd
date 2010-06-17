AC_INIT([ngfd], [0.12])
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
		yes) debug=true ; CFLAGS="${CFLAGS} -DENABLE_DEBUG" ;;
		no)  debug=false ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
	esac],
	[debug=false])
AM_CONDITIONAL([DEBUG], [test x$debug = xtrue])

AC_ARG_ENABLE([tonemapper],
	AS_HELP_STRING([--disable-tonemapper],[Disable tonemapper @<:@default=false@:>@]),
	[case "${enableval}" in
		yes) tonemapper=true ;;
		no)  tonemapper=false ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --disable-tonemapper]) ;;
	esac],
	[tonemapper=true])
AM_CONDITIONAL([TONEMAPPER], [test x$tonemapper = xtrue])

AC_ARG_ENABLE([vibrator],
	AS_HELP_STRING([--disable-vibrator],[Disable vibrator @<:@default=false@:>@]),
	[case "${enableval}" in
		yes) vibrator=true ;;
		no)  vibrator=false ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --disable-vibrator]) ;;
	esac],
	[vibrator=true])
AM_CONDITIONAL([VIBRATOR], [test x$vibrator = xtrue])

AC_ARG_ENABLE([backlight],
	AS_HELP_STRING([--disable-backlight],[Disable backlight @<:@default=false@:>@]),
	[case "${enableval}" in
		yes) backlight=true ;;
		no)  backlight=false ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --disable-backlight]) ;;
	esac],
	[backlight=true])
AM_CONDITIONAL([BACKLIGHT], [test x$backlight = xtrue])

AC_ARG_ENABLE([led],
	AS_HELP_STRING([--disable-led],[Disable led @<:@default=false@:>@]),
	[case "${enableval}" in
		yes) led=true ;;
		no)  led=false ;;
		*) AC_MSG_ERROR([bad value ${enableval} for --disable-led]) ;;
	esac],
	[led=true])
AM_CONDITIONAL([LED], [test x$led = xtrue])

PKG_CHECK_MODULES(CHECK, check)
AC_SUBST(CHECK_LIBS)
AC_SUBST(CHECK_CFLAGS)

PKG_CHECK_MODULES(NGFD,
		  [glib-2.0
		   gobject-2.0
		   gthread-2.0
		   gio-2.0
		   dbus-1
		   dbus-glib-1
		   libpulse
		   libpulse-mainloop-glib
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
    PKG_CHECK_MODULES(GST, [gstreamer-0.10 gio-2.0 gstreamer-controller-0.10],
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

#### Profile support (optional) ####

AC_ARG_ENABLE([profile],
    AS_HELP_STRING([--disable-profile],[Disable optional profile support]),
        [
            case "${enableval}" in
                yes) profile=true ;;
                no) profile=false ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --disable-profile) ;;
            esac
        ],
        [profile=true])

if test "x${profile}" != xfalse ; then
    PKG_CHECK_MODULES(PROFILE, [profile],
        [
            HAVE_PROFILE=1
            AC_DEFINE([HAVE_PROFILE], 1, [Have profile?])
        ],
        [
            HAVE_PROFILE=0
            if test "x$profile" = xtrue ; then
                AC_MSG_ERROR([*** Needed Profile support not found])
            fi
        ])
else
    HAVE_PROFILE=0
fi

#### Stream restore support (optional) ####

AC_ARG_ENABLE([streamrestore],
    AS_HELP_STRING([--disable-streamrestore],[Disable optional stream-restore support]),
        [
            case "${enableval}" in
                yes) streamrestore=true ;;
                no) streamrestore=false ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --disable-streamrestore) ;;
            esac
        ],
        [streamrestore=true])

if test "x${streamrestore}" != xfalse ; then
    HAVE_STREAMRESTORE=1
    AC_DEFINE([HAVE_STREAMRESTORE], 1, [Have stream-restore?])
else
    HAVE_STREAMRESTORE=0
fi

AC_SUBST(PROFILE_CFLAGS)
AC_SUBST(PROFILE_LIBS)
AC_SUBST(HAVE_PROFILE)
AM_CONDITIONAL([HAVE_PROFILE], [test "x$HAVE_PROFILE" = x1])

#### Timestamp support (optional) ####

AC_ARG_ENABLE([timestamp],
    AS_HELP_STRING([--enable-timestamp],[Enable optional timestamping support]),
        [
            case "${enableval}" in
                yes) timestamp=true ;;
                no) timestamp=false ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-timestamp) ;;
            esac
        ],
        [timestamp=false])

if test "x${timestamp}" != xfalse ; then
    PKG_CHECK_MODULES(TIMESTAMP, [sp_timestamp],
        [
            HAVE_TIMESTAMP=1
            AC_DEFINE([HAVE_TIMESTAMP], 1, [Have sp-timestamp?])
        ],
        [
            HAVE_TIMESTAMP=0
            if test "x$timestamp" = xtrue ; then
                AC_MSG_ERROR([*** Needed sp-timestamp support not found])
            fi
        ])
else
    HAVE_TIMESTAMP=0
fi

AC_SUBST(TIMESTAMP_CFLAGS)
AC_SUBST(TIMESTAMP_LIBS)
AC_SUBST(HAVE_TIMESTAMP)
AM_CONDITIONAL([HAVE_TIMESTAMP], [test "x$HAVE_TIMESTAMP" = x1])

AC_SUBST(NGFD_LIBS)
AC_SUBST(NGFD_CFLAGS)

echo "
 == $PACKAGE_NAME $VERSION ==

    Compiler:               ${CC}
    CFLAGS:                 ${CFLAGS}

    Debug enabled:          ${debug}
    Timestamping enabled:   ${timestamp}
    Vibrator enabled:       ${vibrator}
    Backlight enabled:      ${backlight}
    Led enabled:            ${led}
    Tone mapper enabled:    ${tonemapper}
    Stream restore enabled: ${streamrestore}
    Gstreamer enabled:      ${gstreamer}
    Profile enabled:        ${profile}
"

AC_OUTPUT(Makefile src/Makefile tests/Makefile data/Makefile)
