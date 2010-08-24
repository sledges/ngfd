#include <stdlib.h>
#include <check.h>

#include "backlight.c"

START_TEST (test_display_ON)
{
	DBusConnection *connection = NULL;
	fail_unless (backlight_display_on (connection) == FALSE);
	
}
END_TEST

START_TEST (test_prevent_blank)
{
	DBusConnection *connection = NULL;
	fail_unless (backlight_prevent_blank (connection) == FALSE);
	
}
END_TEST

START_TEST (test_cancel_prevent)
{
	DBusConnection *connection = NULL;
	fail_unless (backlight_cancel_prevent_blank (connection) == FALSE);
	
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

	s = suite_create ("Backlight");

	tc = tcase_create ("test display");
	tcase_add_test (tc, test_display_ON);
	suite_add_tcase (s, tc);

	tc = tcase_create ("test prevent blank");
	tcase_add_test (tc, test_prevent_blank);
	suite_add_tcase (s, tc);

	tc = tcase_create ("test cancel prevent blank");
	tcase_add_test (tc, test_cancel_prevent);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
