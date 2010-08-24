#include <stdlib.h>
#include <check.h>
#include <stdio.h>

#include "volume.h"

START_TEST (test_create)
{
	Volume *volume = NULL;
	volume = volume_new ();
	fail_unless (volume != NULL);
	volume_free (volume);
	volume = NULL;
}
END_TEST

START_TEST (test_equals)
{
	Volume *volA = NULL;
	Volume *volB = NULL;
	gboolean result = volume_equals (volA, volB);	// volumes are NULL
	fail_unless (result == FALSE);
	volA = volume_new ();
	fail_unless (volA != NULL);
	volB = volume_new ();
	fail_unless (volB != NULL);

	volA->type = VOLUME_TYPE_FIXED;
	volB->type = VOLUME_TYPE_PROFILE;
	result = volume_equals (volA, volB);		// different types
	fail_unless (result == FALSE);
	
	volB->type = VOLUME_TYPE_FIXED;
	volA->level = 10;
	volB->level = 10;
	result = volume_equals (volA, volB);		// same types, levels
	fail_unless (result == TRUE);
	
	volB->level = 11;
	result = volume_equals (volA, volB);		// same types, different levels
	fail_unless (result == FALSE);
	
	volA->type = VOLUME_TYPE_PROFILE;
	volB->type = VOLUME_TYPE_PROFILE;
	volA->key = NULL;
	volB->key = "a";
	result = volume_equals (volA, volB);		// same types, different keys
	fail_unless (result == FALSE);

	volA->key = "a";
	volA->profile = NULL;
	volB->profile = NULL;
	result = volume_equals (volA, volB);		// same keys, profiles are NULL
	fail_unless (result == TRUE);

	volA->profile = "a";
	volB->profile = "b";
	result = volume_equals (volA, volB);		// same keys, different profiles
	fail_unless (result == FALSE);

	volB->profile = "a";
	result = volume_equals (volA, volB);		// same keys, same profiles
	fail_unless (result == TRUE);

	volA->type = VOLUME_TYPE_LINEAR;
	volB->type = VOLUME_TYPE_LINEAR;
	volA->linear[0] = 10;
	volA->linear[1] = 20;
	volA->linear[2] = 30;
	volB->linear[0] = 10;
	volB->linear[1] = 20;
	volB->linear[2] = 10;
	
	result = volume_equals (volA, volB);
	fail_unless (result == FALSE);
	volB->linear[2] = 30;
	result = volume_equals (volA, volB);
	fail_unless (result == TRUE);

	volA->key = NULL;
	volA->profile = NULL;
	volA->role = NULL;
	volB->key = NULL;
	volB->profile = NULL;
	volB->role = NULL;
	volume_free (volA);
	volume_free (volB);
	volA = NULL;
	volB = NULL;
}
END_TEST

START_TEST (test_generate_role)
{
	Volume *volume = NULL;
	volume = volume_new ();
	fail_unless (volume != NULL);

	volume->role = "ngf";
	gboolean result = volume_generate_role (volume);
	fail_unless (result == FALSE);

	volume->role = NULL;
	volume->type = 10;
	result = volume_generate_role (volume);
	fail_unless (result == FALSE);
	
	volume->type = VOLUME_TYPE_FIXED;
	result = volume_generate_role (volume);
	fail_unless (result == TRUE);
	fail_unless (strncmp (volume->role, "x-meego-fixed-", 14) == 0);

	g_free (volume->role);
	volume->role = NULL;
	volume->type = VOLUME_TYPE_PROFILE;
	result = volume_generate_role (volume);
	fail_unless (result == TRUE);
	fail_unless (strncmp (volume->role, "x-meego-", 8) == 0);
	
	g_free (volume->role);
	volume->role = NULL;
	volume->type = VOLUME_TYPE_LINEAR;
	result = volume_generate_role (volume);
	fail_unless (result == TRUE);
	fail_unless (strcmp (volume->role, "x-meego-linear") == 0);

	volume_free (volume);
	volume = NULL;
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

	s = suite_create ("Volume");

	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Volume equals");
	tcase_add_test (tc, test_equals);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Generate role");
	tcase_add_test (tc, test_generate_role);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
