#include <stdlib.h>
#include <check.h>

#include "event.h"
#include "context.h"
#include "request.h"

START_TEST (test_create)
{
	Request *req = NULL;
	Context *context = NULL;
	Event *event = NULL;

	req = request_new (context, event);
	fail_unless (req == NULL);
	
	context = g_new0 (Context, 1);
	event = event_new ();
	
	req = request_new (context, event);
	fail_unless (req != NULL);

	request_free (req);
	event_free (event);
	g_free (context);
	req = NULL;
	event = NULL;
	context = NULL;
}
END_TEST

START_TEST (test_custom_sound)
{
	Context *context = g_new0 (Context, 1);
	Event *event = event_new ();
	Request *req = request_new (context, event);
	fail_unless (req != NULL);

	request_set_custom_sound (req, NULL);
	fail_unless (req->custom_sound == NULL);
       	
	// path to file, ofcourse it is not sounds like file
	const char *path = "./test-request.c";
	request_set_custom_sound (req, path);
	fail_unless (req->custom_sound != NULL);
	fail_unless (req->custom_sound->type == SOUND_PATH_TYPE_FILENAME);
	fail_unless (strcmp (req->custom_sound->filename, path) == 0 );
	
	g_free (context);
	event_free (event);
	request_free (req);
	context = NULL;
	event = NULL;
	req = NULL;
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

	s = suite_create ("Request");

	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Set custom sound");
	tcase_add_test (tc, test_custom_sound);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
