#include <stdlib.h>
#include <check.h>

#include "event.h"

START_TEST (test_create)
{
	Event *event = NULL;
	event_free (event);
	
	event = event_new ();

	fail_unless (event != NULL);
	event_free (event);
	event = NULL;
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

	s = suite_create ("Event");

	tc = tcase_create ("Create and free");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
