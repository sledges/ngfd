#include <stdlib.h>
#include <check.h>

#include "context.h"
#include "volume-controller.h"
#include "volume.h"

START_TEST (test_create)
{
	Context *context = NULL;
	int result= volume_controller_create (context);
	fail_unless (result == 0);

	context = g_new0 (Context, 1);
	result = volume_controller_create (context);
	fail_unless (result == 1);
	fail_unless (context->volume_queue != NULL);

	volume_controller_destroy (context);
	g_free (context);
	context = NULL;
}
END_TEST

START_TEST (test_update)
{
	Context *context = NULL;
	Volume *volume = NULL;
	int result = volume_controller_update (context, volume);
	fail_unless (result == 0);

	context = g_new0(Context, 1);
	volume = volume_new ();
	result = volume_controller_update (context, volume);
	fail_unless (result == 0);
	
	volume->type = VOLUME_TYPE_LINEAR;
	fail_unless (volume_generate_role (volume) == TRUE);

//	result = volume_controller_update (context, volume);
//	fail_unless (result == 1);
	
	volume_free (volume);
	g_free(context);
	volume = NULL;
	context = NULL;
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

	s = suite_create ("Volume controller");

	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Update");
	tcase_add_test (tc, test_update);
	suite_add_tcase (s, tc);
	
	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
