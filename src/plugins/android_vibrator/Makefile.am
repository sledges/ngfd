plugindir = @NGFD_PLUGIN_DIR@
plugin_LTLIBRARIES = libngfd_android_vibrator.la
libngfd_android_vibrator_la_SOURCES = plugin.c
libngfd_android_vibrator_la_LIBADD = @NGFD_PLUGIN_LIBS@ -lvibrator
libngfd_android_vibrator_la_LDFLAGS = -module -avoid-version
libngfd_android_vibrator_la_CFLAGS = @NGFD_PLUGIN_CFLAGS@ -I$(top_srcdir)/src/include $(ANDROID_VIBRATOR_CFLAGS)

