TESTS = test-profile
check_PROGRAMS = test-profile

INCLUDES = -I$(top_srcdir)/src

test_profile_SOURCES = test-profile.c $(top_srcdir)/src/ngf-profile.c
test_profile_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_profile_LDADD = @CHECK_LIBS@ @NGFD_LIBS@
