plugindir = @NGFD_PLUGIN_DIR@
plugin_LTLIBRARIES = libngfd_fake.la
libngfd_fake_la_SOURCES = plugin.c
libngfd_fake_la_LIBADD = @NGFD_PLUGIN_LIBS@
libngfd_fake_la_LDFLAGS = -module -avoid-version
libngfd_fake_la_CFLAGS = @NGFD_PLUGIN_CFLAGS@ -I$(top_srcdir)/src
