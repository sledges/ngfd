#include <stdlib.h>
#include <check.h>
#include <stdio.h>

#include "definition.h"

START_TEST (test_create)
{
	Definition *def = NULL;
	definition_free (def);
	
	def = definition_new ();

	fail_unless (def != NULL);
	definition_free (def);
	def = NULL;
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

	s = suite_create ("Definition");

	tc = tcase_create ("Create and free");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
