bin_PROGRAMS = ngfd

ngfd_CFLAGS = $(NGFD_CFLAGS) $(GST_CFLAGS) $(DBUS_CFLAGS) $(IMMVIBE_CFLAGS)
ngfd_LDFLAGS = $(NGFD_LIBS) $(GST_LIBS) $(DBUS_LIBS) $(IMMVIBE_LIBS)

ngfd_SOURCES = \
	main.c \
	context.h \
	context.c \
	settings.h \
	settings.c \
	volume-controller.h \
	volume-controller.c \
	properties.h \
	properties.c \
	profile.h \
	dbus-if.h \
	dbus-if.c \
	session.h \
	session.c \
	state.h \
	state.c \
	property.h \
	property.c \
	definition.h \
	definition.c \
	event.h \
	event.c \
	sound-path.h \
	sound-path.c \
	volume.h \
	volume.c \
	vibration-pattern.h \
	vibration-pattern.c \
	request.h \
	request.c \
	player.h \
	player.c \
	audio.h \
	audio.c \
	audio-interface.h \
	audio-interface.c \
	vibrator.h \
	tone-generator.h \
	tone-generator.c \
	backlight.h \
	led.h \
	log.h \
	log.c \
	tone-mapper.h \
	timestamp.h \
	audio-gstreamer.h \
	audio-gstreamer.c

if BUILD_PROFILE
ngfd_CFLAGS += $(PROFILE_CFLAGS)
ngfd_LDFLAGS += $(PROFILE_LIBS)
ngfd_SOURCES += profile.c
else
ngfd_SOURCES += profile-dummy.c
endif

if BUILD_IMMVIBE
ngfd_CFLAGS += $(IMMVIBE_CFLAGS)
ngfd_LDFLAGS += $(IMMVIBE_LIBS)
ngfd_SOURCES += vibrator.c
else
ngfd_SOURCES += vibrator-dummy.c
endif

if BUILD_MCE
ngfd_CFLAGS += $(MCE_CFLAGS)
ngfd_LDFLAGS += $(MCE_LIBS)
ngfd_SOURCES += backlight.c led.c
else
ngfd_SOURCES += backlight-dummy.c led-dummy.c
endif

if TONEMAPPER
ngfd_SOURCES += tone-mapper.c
else
ngfd_SOURCES += tone-mapper-dummy.c
endif
