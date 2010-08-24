#include <stdlib.h>
#include <check.h>

#include "tone-generator.h"

START_TEST (test_tone_gen)
{
	DBusConnection *connection = NULL;
	fail_unless (tone_generator_start (connection, 0) == FALSE);
	fail_unless (tone_generator_stop (connection, 0) == FALSE);
			
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

	s = suite_create ("Tone-generator");

	tc = tcase_create ("Start-stop");
	tcase_add_test (tc, test_tone_gen);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
