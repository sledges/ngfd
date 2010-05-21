#include <stdlib.h>
#include <string.h>
#include <check.h>
#include <glib.h>

#include "profile.h"

START_TEST (test_parse_profile_key)
{
    gchar *out_profile = NULL, *out_key = NULL;
    gboolean ret;

    ret = profile_parse_profile_key (NULL, &out_profile, &out_key);
    fail_unless (ret == FALSE);

    ret = profile_parse_profile_key ("ringing.alert.tone", &out_profile, &out_key);
    fail_unless (ret != FALSE);
    fail_unless (out_profile == NULL);
    fail_unless (strcmp (out_key, "ringing.alert.tone") == 0);

    g_free (out_profile);
    out_profile = NULL;

    ret = profile_parse_profile_key ("ringing.alert.tone@general", &out_profile, &out_key);
    fail_unless (ret == TRUE);
    fail_unless (strcmp (out_profile, "general") == 0);
    fail_unless (strcmp (out_key, "ringing.alert.tone") == 0);

    g_free (out_profile);
    out_profile = NULL;
    g_free (out_key);
    out_key = NULL;

    ret = profile_parse_profile_key ("@random", &out_profile, &out_key);
    fail_unless (ret == FALSE);
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

    s = suite_create ("Profile");

    tc = tcase_create ("Parse profile key");
    tcase_add_test (tc, test_parse_profile_key);
    suite_add_tcase (s, tc);

    sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    num_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
