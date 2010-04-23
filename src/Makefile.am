bin_PROGRAMS = ngfd

ngfd_CFLAGS = $(NGFD_CFLAGS)
ngfd_LDFLAGS = $(NGFD_LIBS)

ngfd_SOURCES = \
	main.c \
	ngf-daemon.h \
	ngf-daemon.c \
	ngf-daemon-settings.c \
	ngf-properties.h \
	ngf-properties.c \
	ngf-profile.h \
	ngf-dbus.h \
	ngf-dbus.c \
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
	ngf-audio-pulse.h \
	ngf-audio-pulse.c \
	ngf-controller.h \
	ngf-controller.c \
	ngf-vibrator.h \
	ngf-tonegen.h \
	ngf-tonegen.c \
	ngf-backlight.h \
	ngf-led.h \
	ngf-log.h \
	ngf-log.c \
	ngf-tone-mapper.h \
	ngf-pulse-context.h \
	ngf-pulse-context.c
	
if HAVE_GST
ngfd_CFLAGS += $(GST_CFLAGS)
ngfd_LDFLAGS += $(GST_LIBS)
ngfd_SOURCES += ngf-audio-gstreamer.h \
	ngf-audio-gstreamer.c 
endif

if HAVE_PROFILE
ngfd_CFLAGS += $(PROFILE_CFLAGS)
ngfd_LDFLAGS += $(PROFILE_LIBS)
ngfd_SOURCES += ngf-profile.c
else
ngfd_SOURCES += ngf-profile-dummy.c
endif

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

if BACKLIGHT
ngfd_SOURCES += ngf-backlight.c
else
ngfd_SOURCES += ngf-backlight-dummy.c
endif

if LED
ngfd_SOURCES += ngf-led.c
else
ngfd_SOURCES += ngf-led-dummy.c
endif
