#include <stdlib.h>
#include <check.h>
#include <stdio.h>

#include "context.h"

START_TEST (test_add_sound_path)
{
	Context *context = NULL;
	fail_unless ( (context = g_new0 (Context, 1)) != NULL);
	SoundPath *soundPath = NULL;
	fail_unless ( (context_add_sound_path (context, soundPath)) == NULL);
	soundPath = sound_path_new ();

	// add one (first) SoundPath to context
	soundPath->type = SOUND_PATH_TYPE_FILENAME;
	soundPath->filename = g_strdup ("qwerty");
	SoundPath *ret_sp = NULL;
	ret_sp = context_add_sound_path (context, soundPath);
	fail_unless (ret_sp != NULL);
	fail_unless (context->num_sounds == 1);
	
	// add second SoundPath to context
	SoundPath *soundPath2 = NULL;
	soundPath2 = sound_path_new ();
	soundPath2->type = SOUND_PATH_TYPE_FILENAME;
	soundPath2->filename = g_strdup ("foobar");
	ret_sp = NULL;
	ret_sp = context_add_sound_path (context, soundPath2);
	fail_unless (ret_sp != NULL);
	fail_unless (context->num_sounds == 2);

	// add equal SoundPath to context
	SoundPath *soundPath3 = NULL;
	soundPath3 = sound_path_new ();
	soundPath3->type = soundPath->type;
	soundPath3->filename = g_strdup (soundPath->filename);
	ret_sp = NULL;
	ret_sp = context_add_sound_path (context, soundPath3);
	fail_unless (ret_sp != NULL);
	fail_unless (context->num_sounds == 2);
	fail_unless (sound_path_equals (ret_sp, soundPath) == TRUE);
	
	// clean ups
	sound_path_free (soundPath);
	sound_path_free (soundPath2);
	soundPath = NULL;
	soundPath2 = NULL;
	soundPath3 = NULL;
	g_free (context->sounds);
	g_free (context);
	ret_sp = NULL;
	context = NULL;	
}
END_TEST

START_TEST (test_add_pattern)
{
	Context *context = NULL;
	fail_unless ( (context = g_new0 (Context, 1)) != NULL);

	VibrationPattern *vibPatt = NULL;
	fail_unless ( (context_add_pattern (context, vibPatt)) == NULL);
	vibPatt = vibration_pattern_new ();

	// add one (first) VibrationPattern to context
	vibPatt->type = VIBRATION_PATTERN_TYPE_INTERNAL;
	vibPatt->pattern = 100;
	VibrationPattern *ret_vp = NULL;
	ret_vp = context_add_pattern (context, vibPatt);
	fail_unless (ret_vp != NULL);
	fail_unless (context->num_patterns == 1);

	// add second VibrationPattern to context
	VibrationPattern *vibPatt2 = vibration_pattern_new ();
	vibPatt2->type = VIBRATION_PATTERN_TYPE_INTERNAL;
	vibPatt2->pattern = 50;
	ret_vp = NULL;
	ret_vp = context_add_pattern (context, vibPatt2);
	fail_unless (ret_vp != NULL);
	fail_unless (context->num_patterns == 2);
	
	
	VibrationPattern *vibPatt3 = vibration_pattern_new ();
	vibPatt3->type = vibPatt->type;
	vibPatt3->pattern = vibPatt->pattern;
	ret_vp = NULL;
	ret_vp = context_add_pattern (context, vibPatt3);
	fail_unless (ret_vp != NULL);
	fail_unless (context->num_patterns == 2);
	fail_unless (vibration_pattern_equals (ret_vp, vibPatt) == TRUE);
	
	vibration_pattern_free (vibPatt);
	vibration_pattern_free (vibPatt2);
	vibPatt = NULL;
	vibPatt2 = NULL;
	vibPatt3 = NULL;
	g_free (context->patterns);
	g_free (context);
	context = NULL;
	ret_vp = NULL;
}
END_TEST

START_TEST (test_add_volume)
{
	Context *context = NULL;
	fail_unless ( (context = g_new0 (Context, 1)) != NULL);
	Volume *volume = NULL;
	fail_unless ( (context_add_volume (context, volume)) == NULL);
	volume = volume_new ();
	volume->type = 10;
	fail_unless ( (context_add_volume (context, volume)) == NULL);
	
	volume = volume_new ();
	volume->type = VOLUME_TYPE_FIXED;
	volume->level = 10;
	Volume *ret_vol = context_add_volume (context, volume);
	fail_unless (ret_vol != NULL);
	fail_unless (context->num_volumes == 1);

	// przez to usuwa volume, ale nie dekrementuje num_volumes
	// to samo jest dla sound-path i vibration pattern
//	ret_vol = NULL;
//	ret_vol = context_add_volume (context, volume);
//	fail_unless (ret_vol != NULL);
//	printf ("\n\n%s it is.\n\n\n", ret_vol == NULL ? "YES" : "NO");
	
	Volume *volume2 = volume_new ();
	volume2->type = VOLUME_TYPE_FIXED;
	volume2->level = 5;
	ret_vol = NULL;
	ret_vol = context_add_volume (context, volume2);
	fail_unless (ret_vol != NULL);
	fail_unless (context->num_volumes == 2);

	Volume *volume3 = volume_new ();
	volume3->type = volume->type;
	volume3->level = volume->level;
	ret_vol = NULL;
	ret_vol = context_add_volume (context, volume3);
	fail_unless (ret_vol != NULL);
	fail_unless (context->num_volumes == 2);
	fail_unless ( volume_equals (ret_vol, volume) == TRUE);
	
	volume_free (volume);
	volume_free (volume2);
	volume = NULL;
	volume2 = NULL;
	volume3 = NULL;
	g_free (context->volumes);
	g_free (context);
	context = NULL;
	ret_vol = NULL;
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

	s = suite_create ("Context");
	
	tc = tcase_create ("Add sound path");
	tcase_add_test (tc, test_add_sound_path);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Add vibration pattern");
	tcase_add_test (tc, test_add_pattern);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Add volume");
	tcase_add_test (tc, test_add_volume);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	
	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
