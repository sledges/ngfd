bin_PROGRAMS = test-audio

INCLUDES = -I $(top_srcdir)/src

test_audio_CFLAGS = $(NGFD_CFLAGS)
test_audio_LDFLAGS = $(NGFD_LIBS)
test_audio_SOURCES = \
	test-audio.c \
	$(top_srcdir)/src/ngf-audio.h \
	$(top_srcdir)/src/ngf-audio.c \
	$(top_srcdir)/src/ngf-log.h \
	$(top_srcdir)/src/ngf-log.c
