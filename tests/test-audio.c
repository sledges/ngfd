#include <stdlib.h>
#include <check.h>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <gst/gst.h>
#include <gst/controller/gstcontroller.h>
#include <gst/controller/gstinterpolationcontrolsource.h>
#include <gio/gio.h>

#include "audio.h"


START_TEST (test_create)
{
	Audio *audio = NULL;
	audio = audio_create ();
	fail_unless (audio != NULL);
	audio_destroy (audio);
	audio = NULL;
}
END_TEST

START_TEST (test_create_stream)
{
	Audio *audio = NULL;
	AudioStream *stream = NULL;
	stream = audio_create_stream (audio, AUDIO_STREAM_NONE);
	fail_unless (stream == NULL);
	
	audio = audio_create ();
	fail_unless (audio != NULL);
	stream = audio_create_stream (audio, AUDIO_STREAM_NONE);
	fail_unless (stream != NULL);

	audio_destroy_stream (audio, stream);
	audio_destroy (audio);
	
	stream = NULL;
	audio = NULL;
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

	g_type_init ();

	s = suite_create ("Audio");

	tc = tcase_create ("Create audio");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Create audio stream");
	tcase_add_test (tc, test_create_stream);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
