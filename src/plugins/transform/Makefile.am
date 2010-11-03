plugindir = @NGFD_PLUGIN_DIR@
plugin_LTLIBRARIES = libngfd_transform.la
libngfd_transform_la_SOURCES = plugin.c
libngfd_transform_la_LIBADD = @NGFD_PLUGIN_LIBS@
libngfd_transform_la_LDFLAGS = -module -avoid-version
libngfd_transform_la_CFLAGS = @NGFD_PLUGIN_CFLAGS@ -I$(top_srcdir)/src/include

