#include <stdlib.h>
#include <check.h>

#include "vibration-pattern.h"

START_TEST (test_create)
{
	VibrationPattern *vibPattern = NULL;
	vibPattern = vibration_pattern_new ();
	fail_unless (vibPattern != NULL);
	vibration_pattern_free (vibPattern);
	vibPattern = NULL;
}
END_TEST

START_TEST (test_equals)
{
	VibrationPattern *vibPattA = NULL;
	VibrationPattern *vibPattB = NULL;

	gboolean result = TRUE;
	result = vibration_pattern_equals (vibPattA, vibPattB);	// NULL
	fail_unless (result == FALSE);

	vibPattA = vibration_pattern_new ();
	fail_unless (vibPattA != NULL);
	vibPattB = vibration_pattern_new ();
	fail_unless (vibPattB != NULL);

	vibPattA->type = VIBRATION_PATTERN_TYPE_FILENAME;
	vibPattB->type = VIBRATION_PATTERN_TYPE_PROFILE;
	result = TRUE;
	result = vibration_pattern_equals (vibPattA, vibPattB);	// different types
	fail_unless (result == FALSE);

	vibPattB->type = VIBRATION_PATTERN_TYPE_FILENAME;
	vibPattA->filename = "a";
	vibPattB->filename = "a";
	vibPattA->pattern = 1;
	vibPattB->pattern = 1;
	result = vibration_pattern_equals (vibPattA, vibPattB);	// same types, filenames, patterns
	fail_unless (result == TRUE);

	vibPattB->filename = "b";
	result = vibration_pattern_equals (vibPattA, vibPattB); // different filnames
	fail_unless (result == FALSE);

	vibPattB->filename = "a";
	vibPattB->pattern = 2;
	result = vibration_pattern_equals (vibPattA, vibPattB); // different patterns
	fail_unless (result == FALSE);
	
	vibPattA->filename = NULL;
	result = vibration_pattern_equals (vibPattA, vibPattB); // same types, one filename = NULL
	fail_unless (result == FALSE);
	
	vibPattA->type = VIBRATION_PATTERN_TYPE_PROFILE;
	vibPattB->type = VIBRATION_PATTERN_TYPE_PROFILE;
	vibPattA->key = "a";
	vibPattB->key = NULL;
	result = vibration_pattern_equals (vibPattA, vibPattB); // same types, one key == NULL

	vibPattB->key = "a";
	vibPattA->profile = NULL;
	vibPattB->profile = NULL;
	result = vibration_pattern_equals (vibPattA, vibPattB); // both profiles are NULL
	fail_unless (result == TRUE);
	
	vibPattA->profile = "a";
	vibPattB->profile = "b";
	result = vibration_pattern_equals (vibPattA, vibPattB); // different profiles
	fail_unless (result == FALSE);

	vibPattB->profile = "a";
	result = vibration_pattern_equals (vibPattA, vibPattB); // same types, keys, profiles
	fail_unless (result == TRUE);

	vibPattA->type = VIBRATION_PATTERN_TYPE_INTERNAL;
	vibPattB->type = VIBRATION_PATTERN_TYPE_INTERNAL;
	result = vibration_pattern_equals (vibPattA, vibPattB); // same types, different patterns
	fail_unless (result == FALSE);

	vibPattB->pattern = 1;
	result = vibration_pattern_equals (vibPattA, vibPattB); // same types, patterns
	fail_unless (result == TRUE);
	
	vibPattA->filename = NULL;
	vibPattA->key = NULL;
	vibPattA->profile = NULL;
	vibPattB->filename = NULL;
	vibPattB->key = NULL;
	vibPattB->profile = NULL;
	vibration_pattern_free (vibPattA);
	vibration_pattern_free (vibPattB);
	vibPattA = NULL;
	vibPattB = NULL;	
}
END_TEST

int
main (int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	int num_failed = 0;

	Suite *s = NULL;
	TCase *tc = NULL;
	SRunner *sr = NULL;

	s = suite_create ("Vibration-pattern");

	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Vibration patterns equals");
	tcase_add_test (tc, test_equals);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
