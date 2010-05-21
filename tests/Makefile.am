TESTS = \
	test-profile \
	test-controller

check_PROGRAMS = \
	test-profile \
	test-controller

INCLUDES = -I$(top_srcdir)/src

test_profile_SOURCES = test-profile.c $(top_srcdir)/src/profile.c
test_profile_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@ @PROFILE_CFLAGS@
test_profile_LDADD = @CHECK_LIBS@ @NGFD_LIBS@ @PROFILE_LIBS@

test_controller_SOURCES = test-controller.c $(top_srcdir)/src/controller.c $(top_srcdir)/src/log.c
test_controller_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_controller_LDADD = @CHECK_LIBS@ @NGFD_LIBS@
