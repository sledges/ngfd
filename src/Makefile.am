bin_PROGRAMS = ngfd

ngfd_CFLAGS= \
	$(NGFD_CFLAGS)

ngfd_SOURCES = \
	main.c \
	ngf-daemon.h \
	ngf-daemon.c \
	ngf-daemon-settings.c \
	ngf-profile.h \
	ngf-profile.c \
	ngf-dbus.h \
	ngf-dbus.c \
	ngf-conf.h \
	ngf-conf.c \
	ngf-value.h \
	ngf-value.c \
	ngf-event-manager.h \
	ngf-event-manager.c \
	ngf-event-definition.h \
	ngf-event-definition.c \
	ngf-event-prototype.h \
	ngf-event-prototype.c \
	ngf-event.h \
	ngf-event.c \
	ngf-audio.h \
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
	ngf-tone-mapper.c

ngfd_LDFLAGS = \
	$(NGFD_LIBS)

if GSTREAMER
ngfd_SOURCES += ngf-audio-gstreamer.c
ngfd_CFLAGS  += $(GST_CFLAGS)
ngfd_LDFLAGS += $(GST_LIBS)
else
ngfd_SOURCES += ngf-audio.c
endif

if VIBRATOR
ngfd_SOURCES += ngf-vibrator.c
ngfd_LDFLAGS += -limmvibe
else
ngfd_SOURCES += ngf-vibrator-dummy.c
endif
