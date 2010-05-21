bin_PROGRAMS = ngfd

ngfd_CFLAGS = $(NGFD_CFLAGS)
ngfd_LDFLAGS = $(NGFD_LIBS)

ngfd_SOURCES = \
	main.c \
	daemon.h \
	daemon.c \
	settings.c \
	properties.h \
	properties.c \
	profile.h \
	dbus-if.h \
	dbus-if.c \
	state.h \
	state.c \
	property.h \
	property.c \
	definition.h \
	definition.c \
	event-prototype.h \
	event-prototype.c \
	request.h \
	request.c \
	audio.h \
	audio.c \
	audio-interface.h \
	audio-interface.c \
	audio-pulse.h \
	audio-pulse.c \
	controller.h \
	controller.c \
	vibrator.h \
	tone-generator.h \
	tone-generator.c \
	backlight.h \
	led.h \
	log.h \
	log.c \
	tone-mapper.h \
	pulse-context.h \
	pulse-context.c \
	timestamp.h

if HAVE_GST
ngfd_CFLAGS += $(GST_CFLAGS)
ngfd_LDFLAGS += $(GST_LIBS)
ngfd_SOURCES += audio-gstreamer.h \
	audio-gstreamer.c
endif

if HAVE_PROFILE
ngfd_CFLAGS += $(PROFILE_CFLAGS)
ngfd_LDFLAGS += $(PROFILE_LIBS)
ngfd_SOURCES += profile.c
else
ngfd_SOURCES += profile-dummy.c
endif

if HAVE_TIMESTAMP
ngfd_CFLAGS += $(TIMESTAMP_CFLAGS)
ngfd_LDFLAGS += $(TIMESTAMP_LIBS)
endif

if TONEMAPPER
ngfd_SOURCES += tone-mapper.c
else
ngfd_SOURCES += tone-mapper-dummy.c
endif

if VIBRATOR
ngfd_SOURCES += vibrator.c
ngfd_LDFLAGS += -limmvibe
else
ngfd_SOURCES += vibrator-dummy.c
endif

if BACKLIGHT
ngfd_SOURCES += backlight.c
else
ngfd_SOURCES += backlight-dummy.c
endif

if LED
ngfd_SOURCES += led.c
else
ngfd_SOURCES += led-dummy.c
endif
