TESTS = \
	test-controller

check_PROGRAMS = \
	test-controller

INCLUDES = -I$(top_srcdir)/src

test_controller_SOURCES = test-controller.c $(top_srcdir)/src/controller.c $(top_srcdir)/src/log.c
test_controller_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_controller_LDADD = @CHECK_LIBS@ @NGFD_LIBS@
