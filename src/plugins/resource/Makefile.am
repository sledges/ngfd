plugindir = @NGFD_PLUGIN_DIR@
plugin_LTLIBRARIES = libngfd_resource.la
libngfd_resource_la_SOURCES = plugin.c
libngfd_resource_la_LIBADD = @NGFD_PLUGIN_LIBS@ @DBUS_LIBS@
libngfd_resource_la_LDFLAGS = -module -avoid-version
libngfd_resource_la_CFLAGS = @NGFD_PLUGIN_CFLAGS@ @DBUS_CFLAGS@ -I$(top_srcdir)/src/include
