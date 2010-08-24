#include <stdlib.h>
#include <check.h>

#include "sound-path.h"

START_TEST (test_create)
{
	SoundPath *spath = NULL;
	spath = sound_path_new ();
	fail_unless (spath != NULL);
	sound_path_free (spath);
	spath = NULL;
}
END_TEST

START_TEST (test_equals)
{
	SoundPath *a = NULL;
	SoundPath *b = NULL;
	gboolean result = TRUE;
	
	result = sound_path_equals (a, b);	// NULL
	fail_unless (result == FALSE);

	a = sound_path_new ();
	b = sound_path_new ();
	a->type = SOUND_PATH_TYPE_FILENAME;
	a->filename = "ngf";
	b->type = SOUND_PATH_TYPE_PROFILE;
	b->filename = "ngf";
	result = sound_path_equals (a, b);	// diferent types
	fail_unless (result == FALSE);
	b->type = SOUND_PATH_TYPE_FILENAME;
	result = sound_path_equals (a, b);	// both the same filename
	fail_unless (result == TRUE);
	a->type = SOUND_PATH_TYPE_PROFILE;
	b->type = SOUND_PATH_TYPE_PROFILE;
	result = sound_path_equals (a, b);	// keys are NULL
	fail_unless (result == FALSE);
	a->key = "a";
	b->key = "a";
	result = sound_path_equals (a, b);	// keys are the, but profiles are NULL
	fail_unless (result == TRUE);
	a->profile = "b";
	result = sound_path_equals (a, b);	// one profile is NULL
	fail_unless (result == FALSE);
	b->profile = "b";
	result = sound_path_equals (a, b);	// keys and profiles are the same
	fail_unless (result == TRUE);
	b->profile = "a";
	result = sound_path_equals (a, b);	// different profiles - the last return
	fail_unless (result == FALSE);

	a->filename = NULL;
	a->key = NULL;
	a->profile = NULL;
	b->filename = NULL;
	b->key = NULL;
	b->profile = NULL;
	sound_path_free (b);
	sound_path_free (a);
	a = NULL;
	b = NULL;
}
END_TEST

int
main (int argc,char *argv[])
{
	(void) argc;
	(void) argv;

	int num_failed = 0;

	Suite *s = NULL;
	TCase *tc = NULL;
	SRunner *sr = NULL;

	s = suite_create ("Sound-path");
	
	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Path equals");
	tcase_add_test (tc, test_equals);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;	
}
