#include <stdlib.h>
#include <check.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>


#include "context.h"
#include "dbus-if.c"

START_TEST (test_create)
{
	Context *context = NULL;
	fail_unless ( (context = g_new0 (Context, 1)) != NULL);

//	fail_unless ( (context->system_bus = get_dbus_connection (DBUS_BUS_SYSTEM)) != NULL );
	
//	int result = dbus_if_create (context);
//	fail_unless (result == 1);
	g_free (context);
	context = NULL;
}
END_TEST

int
main (int argc, char *argv[])
{
	(void) argc;
	(void) argv;

	int num_failed = 0;

	Suite *s = NULL;
	TCase *tc = NULL;
	SRunner *sr = NULL;

	g_type_init ();

	s = suite_create ("Dbus-if");

	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;	
}
