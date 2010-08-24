#include <stdlib.h>
#include <check.h>

#include "audio-gstreamer.h"
#include "audio-interface.h"

START_TEST (test_create)
{
	AudioInterface *iface = NULL;
	iface = audio_gstreamer_create ();
	gboolean result = audio_interface_initialize (iface);
	fail_unless (result == TRUE);
	fail_unless (iface != NULL);
	
	audio_interface_shutdown (iface);
	iface = NULL;	
}
END_TEST

START_TEST (test_create_stream)
{
	AudioInterface *iface = NULL;
	iface = audio_gstreamer_create ();
	fail_unless (audio_interface_initialize (iface) == TRUE);
	fail_unless (iface != NULL);

	AudioStream *stream = NULL;
	stream = audio_interface_create_stream (iface);
	fail_unless (stream != NULL);

	audio_interface_destroy_stream (NULL, stream);
	fail_unless (stream->iface != NULL);

	stream->properties = pa_proplist_new ();

	audio_interface_destroy_stream (iface, stream);
	stream = NULL;
	audio_interface_shutdown (iface);
	iface = NULL;	
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

	s = suite_create ("Audio interface");

	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Create stream");
	tcase_add_test (tc, test_create_stream);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	
	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;	
}
