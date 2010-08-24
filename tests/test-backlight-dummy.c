#include <stdlib.h>
#include <check.h>

#include "backlight.h"

START_TEST (test_all)
{
	DBusConnection *connection = NULL;
	fail_unless (backlight_display_on (connection) == TRUE);
	fail_unless (backlight_prevent_blank (connection) == TRUE);
	fail_unless (backlight_cancel_prevent_blank (connection) == TRUE);
}
END_TEST

int
main (int argc, char* argv[])
{
	(void) argc;
	(void) argv;

	int num_failed = 0;

	Suite *s = NULL;
	TCase *tc = NULL;
	SRunner *sr = NULL;

	s = suite_create ("Backlight-dummy");

	tc = tcase_create ("test everything");
	tcase_add_test (tc, test_all);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
