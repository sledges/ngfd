#include <stdlib.h>
#include <check.h>

#include "src/include/ngf/request.h"
#include "src/include/ngf/proplist.h"
#include "output.c"

static void
req_free (NRequest *req)
{
	n_request_free (req);
	req = NULL;
}

START_TEST (test_create)
{
	NRequest *request = NULL;
	request = n_request_new ();
	fail_unless (request != NULL);
	fail_unless (n_request_get_name (request) == NULL);
	req_free (request);
	
	const char *str = "event";
	request = n_request_new_with_event (str);
	fail_unless (request != NULL);
	const char *name = n_request_get_name (request);
	fail_unless (g_strcmp0 (name, str) == 0);
	req_free (request);
}
END_TEST

START_TEST (test_create_with_event_and_proplist)
{
    NRequest *request = NULL;
    const char *event = "event";
    const char *key = "key";
    int value = -100;
    NProplist *proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    n_proplist_set_int (proplist, key, value);
    request = n_request_new_with_event_and_properties (NULL, proplist);
    fail_unless (request == NULL);
    request = n_request_new_with_event_and_properties (event, NULL);
    fail_unless (request != NULL);
    fail_unless (n_request_get_properties (request) == NULL);
    req_free (request);
    
    request = n_request_new_with_event_and_properties (event, proplist);
    const NProplist *set = n_request_get_properties (request);
    fail_unless (n_proplist_match_exact (set, proplist) == TRUE);
    const char *name = n_request_get_name (request);
    fail_unless (g_strcmp0 (name, event) == 0);
    
    n_proplist_free (proplist);
    proplist = NULL;
    req_free (request);
}
END_TEST

START_TEST (test_properties)
{
    NRequest *request = NULL;
    request = n_request_new ();
    fail_unless (request != NULL);
    NProplist *proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    int n = 5;
    char **keys = keys_init (n);
    int *values = int_values (n);
    int i = n;
    while (i--)
    {
        n_proplist_set_int (proplist, keys[i], values[i]);
    }
    /* get properties from NULL request */
    fail_unless (n_request_get_properties (NULL) == NULL);
    
    n_request_set_properties (NULL, proplist);
    fail_unless (n_request_get_properties (request) == NULL);
    
    n_request_set_properties (request, NULL);
    fail_unless (n_request_get_properties (request) == NULL);
    
    n_request_set_properties (request, proplist);
    const NProplist *set = n_request_get_properties (request);
    fail_unless (n_proplist_match_exact (set, proplist) == TRUE);

    n_request_set_properties (NULL, proplist);
    set = NULL;
    set = n_request_get_properties (request);
    fail_unless (n_proplist_match_exact (set, proplist) == TRUE);
    n_request_set_properties (request, NULL);
    set = NULL;
    set = n_request_get_properties (request);
    fail_unless (n_proplist_match_exact (set, proplist) == TRUE);
    
    n_proplist_free (proplist);
    proplist = NULL;
    keys_free (keys, n);
    keys = NULL;
    int_values_free (values);
    values = NULL;
    req_free (request);
}
END_TEST

START_TEST (test_data)
{
    NRequest *request = NULL;
    request = n_request_new ();
    fail_unless (request != NULL);
    const char *key = "key";

    fail_unless (n_request_get_data (NULL, key) == NULL);
    fail_unless (n_request_get_data (request, NULL) == NULL);
    fail_unless (n_request_get_data (request, key) == NULL);

    void *data;
    n_request_store_data (NULL, key, request);
    data = n_request_get_data (request, key);
    fail_unless (data == NULL);
    n_request_store_data (request, NULL, request);
    data = n_request_get_data (request, key);
    fail_unless (data == NULL);
    n_request_store_data (request, key, request);
    /* needs verification */

    req_free (request);
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

	s = suite_create ("\tRequest tests");

	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("properties in request - set and get");
	tcase_add_test (tc, test_properties);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Create with event and proplist");
	tcase_add_test (tc, test_create_with_event_and_proplist);
	suite_add_tcase (s, tc);

	tc = tcase_create ("store and get data");
	tcase_add_test (tc, test_data);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
