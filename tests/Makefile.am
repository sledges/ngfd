TESTS = \
       test-value \
       test-request \
       test-event \
       test-proplist       

check_PROGRAMS = \
       test-value \
       test-request \
       test-event \
       test-proplist

INCLUDES = -I$(top_srcdir)/src/include

test_value_SOURCES = test-value.c $(top_srcdir)/src/ngf/value.c $(top_srcdir)/src/ngf/log.c
test_value_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_value_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_request_SOURCES = test-request.c $(top_srcdir)/src/ngf/request.c $(top_srcdir)/src/ngf/proplist.c $(top_srcdir)/src/ngf/value.c $(top_srcdir)/src/ngf/log.c
test_request_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_request_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_event_SOURCES = test-event.c $(top_srcdir)/src/ngf/event.c $(top_srcdir)/src/ngf/proplist.c $(top_srcdir)/src/ngf/value.c $(top_srcdir)/src/ngf/log.c 
test_event_CFLAGS =@CHECK_CFLAGS@ @NGFD_CFLAGS@
test_event_LDADD = @CHECK_LIBS@ @NGFD_LIBS@

test_proplist_SOURCES = test-proplist.c $(top_srcdir)/src/ngf/proplist.c $(top_srcdir)/src/ngf/value.c $(top_srcdir)/src/ngf/log.c
test_proplist_CFLAGS = @CHECK_CFLAGS@ @NGFD_CFLAGS@
test_proplist_LDADD = @CHECK_LIBS@ @NGFD_LIBS@
