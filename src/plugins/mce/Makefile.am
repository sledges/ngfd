plugindir = @NGFD_PLUGIN_DIR@
plugin_LTLIBRARIES = libngfd_mce.la
libngfd_mce_la_SOURCES = plugin.c
libngfd_mce_la_LIBADD = @NGFD_PLUGIN_LIBS@ @DBUS_LIBS@
libngfd_mce_la_LDFLAGS = -module -avoid-version
libngfd_mce_la_CFLAGS = @NGFD_PLUGIN_CFLAGS@ @DBUS_CFLAGS@ -I$(top_srcdir)/src
