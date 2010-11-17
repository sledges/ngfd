plugindir = @NGFD_PLUGIN_DIR@
plugin_LTLIBRARIES = libngfd_gst.la
libngfd_gst_la_SOURCES = plugin.c volume.c
libngfd_gst_la_LIBADD = @NGFD_PLUGIN_LIBS@ @GST_LIBS@
libngfd_gst_la_LDFLAGS = -module -avoid-version
libngfd_gst_la_CFLAGS = @NGFD_PLUGIN_CFLAGS@ @GST_CFLAGS@ -I$(top_srcdir)/src/include
