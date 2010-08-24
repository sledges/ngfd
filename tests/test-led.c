#include <stdlib.h>
#include <check.h>

#include "led.h"

START_TEST (test_led_toggle)
{
	DBusConnection *connection = NULL;
	fail_unless (led_activate_pattern (connection, NULL) == FALSE);
	fail_unless (led_deactivate_pattern (connection, NULL) == FALSE);
			
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

	s = suite_create ("LED");

	tc = tcase_create ("Toggle LED");
	tcase_add_test (tc, test_led_toggle);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
