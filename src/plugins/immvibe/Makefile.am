plugindir = @NGFD_PLUGIN_DIR@
plugin_LTLIBRARIES = libngfd_immvibe.la
libngfd_immvibe_la_SOURCES = plugin.c
libngfd_immvibe_la_LIBADD = @NGFD_PLUGIN_LIBS@ @IMMVIBE_LIBS@
libngfd_immvibe_la_LDFLAGS = -module -avoid-version
libngfd_immvibe_la_CFLAGS = @NGFD_PLUGIN_CFLAGS@ @IMMVIBE_CFLAGS@ -I$(top_srcdir)/src
