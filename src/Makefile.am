bin_PROGRAMS = ngfd

ngfd_CFLAGS= \
	$(NGFD_CFLAGS) $(GST_CFLAGS)

ngfd_SOURCES = \
	main.c \
	ngf-daemon.h \
	ngf-daemon.c \
	ngf-daemon-settings.c \
	ngf-properties.h \
	ngf-properties.c \
	ngf-profile.h \
	ngf-profile.c \
	ngf-dbus.h \
	ngf-dbus.c \
	ngf-conf.h \
	ngf-conf.c \
	ngf-value.h \
	ngf-value.c \
	ngf-event-definition.h \
	ngf-event-definition.c \
	ngf-event-prototype.h \
	ngf-event-prototype.c \
	ngf-event.h \
	ngf-event.c \
	ngf-audio.h \
	ngf-audio.c \
	ngf-audio-interface.h \
	ngf-audio-interface.c \
	ngf-audio-gstreamer.h \
	ngf-audio-gstreamer.c \
	ngf-audio-pulse.h \
	ngf-audio-pulse.c \
	ngf-controller.h \
	ngf-controller.c \
	ngf-vibrator.h \
	ngf-tonegen.h \
	ngf-tonegen.c \
	ngf-backlight.h \
	ngf-backlight.c \
	ngf-led.h \
	ngf-led.c \
	ngf-log.h \
	ngf-log.c \
	ngf-tone-mapper.h \
	ngf-pulse-context.h \
	ngf-pulse-context.c

ngfd_LDFLAGS = \
	$(NGFD_LIBS) $(GST_LIBS)

if TONEMAPPER
ngfd_SOURCES += ngf-tone-mapper.c
else
ngfd_SOURCES += ngf-tone-mapper-dummy.c
endif

if VIBRATOR
ngfd_SOURCES += ngf-vibrator.c
ngfd_LDFLAGS += -limmvibe
else
ngfd_SOURCES += ngf-vibrator-dummy.c
endif
