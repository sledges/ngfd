#include <stdlib.h>
#include <check.h>

#include "src/include/ngf/proplist.h"
#include "output.c"

static void
plist_free (NProplist *list)
{
    n_proplist_free (list);
    list = NULL;
}

START_TEST (test_create)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);

    plist_free (proplist);
}
END_TEST

START_TEST (test_copy)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    NProplist *copy = NULL;
    copy = n_proplist_copy (NULL);
    fail_unless (copy == NULL);
    copy = n_proplist_copy (proplist);
    fail_unless (copy != NULL);
    fail_unless (n_proplist_match_exact (proplist, copy) == TRUE);
    n_proplist_set_int (proplist, "key", 100);
    plist_free (copy);
    copy = n_proplist_copy (proplist);
    fail_unless (copy != NULL);
    fail_unless (n_proplist_match_exact (proplist, copy) == TRUE);
    
    plist_free (copy);
    plist_free (proplist);
}
END_TEST

START_TEST (test_copy_keys)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    NProplist *keys = NULL;
    GList *list = NULL;
    keys = n_proplist_copy_keys (proplist, list);
    fail_unless (keys != NULL);
    fail_unless (n_proplist_is_empty (keys) == TRUE);
    plist_free (keys);
	    
    const char *key1 = "key1";
    const char *key2 = "key2";
    uint val_key2 = 100;
    list = g_list_append (list, (gpointer)key2);

    keys = n_proplist_copy_keys (NULL, list);
    fail_unless (keys != NULL);
    fail_unless (n_proplist_is_empty (keys) == TRUE);
    plist_free (keys);
    keys = n_proplist_copy_keys (proplist, NULL);
    fail_unless (keys != NULL);
    fail_unless (n_proplist_is_empty (keys) == TRUE);
    plist_free (keys);

    keys = n_proplist_copy_keys (proplist, list);
    fail_unless (keys != NULL);
    fail_unless (n_proplist_is_empty (keys) == TRUE);
    n_proplist_set_bool (proplist, key1, FALSE);
    n_proplist_set_uint (proplist, key2, val_key2);
    plist_free (keys);
    keys = n_proplist_copy_keys (proplist, list);
    fail_unless (keys != NULL);
    fail_unless (n_proplist_is_empty (keys) == FALSE);
    fail_unless (n_proplist_size (keys) == 1);
    fail_unless (n_proplist_get_uint (keys, key2) == val_key2);
    
    
    g_list_free (list);
    list = NULL;
    plist_free (keys);
    plist_free (proplist);
}
END_TEST
void listing (const char*, const NValue*, gpointer);

START_TEST (test_merge)
{
    NProplist *source = NULL;
    source = n_proplist_new ();
    fail_unless (source != NULL);
    NProplist *target = NULL;
    target = n_proplist_new ();
    fail_unless (target != NULL);
    int n = 5;
    char **keys = keys_init (n);
    char **s_values = str_values (n);
    int i;
    for (i = 0; i < n; i++)
        n_proplist_set_string (target, keys[i], s_values[i]);

    char *merge_key1 = keys[0];
    char *merge_key2 = keys[3];
    char *key1_value = s_values[3];

    n_proplist_set_string (source, merge_key1, key1_value);
    n_proplist_set_int (source, merge_key2, n);
    char *new_key = "diff";
    n_proplist_set_bool (source, new_key, TRUE);

    NProplist *targetCopy = n_proplist_copy (target);
    NProplist *sourceCopy = n_proplist_copy (source);
    
    n_proplist_merge (NULL, source);
    fail_unless (n_proplist_has_key (target, new_key) == FALSE);
    fail_unless (n_proplist_match_exact (target, targetCopy) == TRUE);
    fail_unless (n_proplist_match_exact (source, sourceCopy) == TRUE);
    
    n_proplist_merge (target, NULL);
    fail_unless (n_proplist_has_key (target, new_key) == FALSE);
    fail_unless (n_proplist_match_exact (target, targetCopy) == TRUE);
    fail_unless (n_proplist_match_exact (source, sourceCopy) == TRUE);
    

    /* Two keys are common (will be replaces),
     * three will be added from source,
     * one will stay from target
     */
    n_proplist_merge (target, source);
//    n_proplist_foreach (target, listing, NULL);
    
    fail_unless (n_proplist_match_exact (target, targetCopy) == FALSE);
    fail_unless (n_proplist_match_exact (source, sourceCopy) == TRUE);
    fail_unless (n_proplist_size (target) == n + 1);
    const char *merged1 = n_proplist_get_string (target, merge_key1);
    fail_unless (g_strcmp0 (key1_value, merged1) == 0);
    int merged2 = n_proplist_get_int (target, merge_key2);
    fail_unless (merged2 == n);
    
    g_free ((char *)merged1);   
    str_values_free (s_values, n);
    keys_free (keys, n);
    plist_free (sourceCopy);
    plist_free (targetCopy);
    plist_free (source);
    plist_free (target);
}
END_TEST

void 
listing (const char *key, const NValue *value, gpointer userdata)
{
	(void) userdata;
	char *string_value = n_value_to_string ((NValue*)value);
	printf ("\nKey = %s\t|\tValue = %s\n\n", key, string_value);
	g_free (string_value);
}


START_TEST (test_merge_keys)
{
    NProplist *source = NULL;
    source = n_proplist_new ();
    fail_unless (source != NULL);   
    NProplist *target = NULL;
    target = n_proplist_new ();
    fail_unless (target != NULL);
    /* total amount of entries - amount of entries in target proplist*/
    int n = 12;
    /* amount of entries in source proplist*/
    int n_second = n / 2;
    /* amount of elements in GList */
    int n_list = n_second / 2;
    /* n keys */
    char **keys = keys_init (n);
    /* n string values for target proplist */
    char **s_values = str_values (n);
    /* n_second int values for source proplist */
    int *i_values = int_values (n_second);
    int i;
    for (i = 0; i < n; i++)
    {
        n_proplist_set_string (target, keys[i], s_values[i]);
	if (i < n_second)
            n_proplist_set_int (source, keys[i], i_values[i]);
    }

    GList *list = NULL;
    for (i = 0; i < n_list; i++)
    {
        list = g_list_append (list, (gpointer)keys[i]);
    }

    /* copy of proplist for comparision */
    NProplist *sourceCopy = n_proplist_copy (source);
    NProplist *targetCopy = n_proplist_copy (target);
    
    n_proplist_merge_keys (NULL, source, list);
    fail_unless (n_proplist_match_exact (source, sourceCopy) == TRUE);
    fail_unless (n_proplist_match_exact (target, targetCopy) == TRUE);
    
    n_proplist_merge_keys (target, NULL, list);
    fail_unless (n_proplist_match_exact (source, sourceCopy) == TRUE);
    fail_unless (n_proplist_match_exact (target, targetCopy) == TRUE);
    
    /* copy of target proplist to check merge_keys with NULL GList = normal merge */
    NProplist *secondTarget = n_proplist_copy (target);
    /* first half of seconfTarget will be fill with ints from source */
    n_proplist_merge_keys (secondTarget, source, NULL);
//    n_proplist_foreach (secondTarget, listing, NULL);
    fail_unless (n_proplist_match_exact (target, secondTarget) == FALSE);
    for (i = 0; i < n; i++)
    {
        if (i < n_second)
        {
            int result = n_proplist_get_int (secondTarget, keys[i]);
            fail_unless (result == i_values[i]);
        }
        else
        {
            const char *result = n_proplist_get_string (secondTarget, keys[i]);
            fail_unless (g_strcmp0 (result, s_values[i]) == 0);
        }
    }
    
    /* half elements from source will be transfer to the target 
     * first 1/4 of the elements from target will be int type */
    n_proplist_merge_keys (target, source, list);
//    n_proplist_foreach (target, listing, NULL);
    fail_unless (n_proplist_match_exact (target, targetCopy) == FALSE);
    fail_unless (n_proplist_match_exact (source, sourceCopy) == TRUE);

    for (i = 0; i < n; i++)
    {
        if (i < n_list)
	{
            int result = n_proplist_get_int (target, keys[i]);
            fail_unless (result == i_values[i]);
	}
	else
	{
            const char *result = n_proplist_get_string (target, keys[i]);
	    fail_unless (g_strcmp0 (result, s_values[i]) == 0);
	}
    }
    
    str_values_free (s_values, n);
    int_values_free (i_values);
    keys_free (keys, n);
    plist_free (secondTarget);
    plist_free (targetCopy);
    plist_free (sourceCopy);
    plist_free (target);
    plist_free (source);
}
END_TEST

START_TEST (test_size)
{
    const char *key = "ngf";
    NProplist *proplist = NULL;
    int size = n_proplist_size (proplist);
    fail_unless (size == 0);
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    size = n_proplist_size (proplist);
    fail_unless (size == 0);
    n_proplist_set_pointer (proplist, key, proplist);
    size = n_proplist_size (proplist);
    fail_unless (size == 1);

    plist_free (proplist);
}
END_TEST

void
n_proplist_Callback (const char *key, const NValue *value, gpointer data)
{
    (void) key;
    (void) value;
    int *check = data;
    (*check)++;
}

START_TEST (test_foreach)
{
   NProplist *proplist = NULL;
   proplist = n_proplist_new ();
   fail_unless (proplist != NULL);
   int n = 10;
   int check_n = 0;
   char **keys = keys_init (n);
   uint *values = uint_values (n);
   int i;
   for (i = 0; i < n; i++)
   {
       n_proplist_set_uint (proplist, keys[i], values[i]);
   }
   n_proplist_foreach (proplist, n_proplist_Callback, &check_n);
   fail_unless (check_n == n);

   uint_values_free (values);
   keys_free (keys, n);
   plist_free (proplist);
}
END_TEST

START_TEST (test_is_empty)
{
    const char *key = "ngf";
    NProplist *proplist = NULL;
    gboolean result = n_proplist_is_empty (proplist);
    fail_unless (result == FALSE);
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    result = n_proplist_is_empty (proplist);
    fail_unless (result == TRUE);
    n_proplist_set_pointer (proplist, key, proplist);
    result = n_proplist_is_empty (proplist);
    fail_unless (result == FALSE);

    plist_free (proplist);
}
END_TEST

START_TEST (test_has_key)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    const char *key = "key";
    gboolean result = n_proplist_has_key (proplist, key);
    fail_unless (result == FALSE);
    result = n_proplist_has_key (NULL, key);
    fail_unless (result == FALSE);
//    result = n_proplist_has_key (proplist, NULL);
//    fail_unless (result == FALSE);
    
    n_proplist_set_int (proplist, key, 100);
    result = n_proplist_has_key (proplist, key);
    fail_unless (result == TRUE);

    plist_free (proplist);
}
END_TEST

START_TEST (test_match_exact)
{
	NProplist *proplistA = NULL;
	proplistA = n_proplist_new ();
	fail_unless (proplistA != NULL);
	NProplist *proplistB = NULL;
	proplistB = n_proplist_new ();
	fail_unless (proplistB != NULL);
	const char* key1 = "key1";
	const char* key2 = "key2";

	gboolean result = n_proplist_match_exact (NULL, proplistB);
	fail_unless (result == FALSE);
	result = n_proplist_match_exact (proplistA, NULL);
	fail_unless (result == FALSE);

	n_proplist_set_int (proplistA, key1, 100);
	result = n_proplist_match_exact (proplistA, proplistB);
	fail_unless (result == FALSE);
	n_proplist_set_int (proplistB, key1, 100);
	result = n_proplist_match_exact (proplistA, proplistB);
	fail_unless (result == TRUE);

	n_proplist_set_bool (proplistA, key2, TRUE);
	n_proplist_set_bool (proplistB, key2, FALSE);
	result = n_proplist_match_exact (proplistA, proplistB);
	fail_unless (result == FALSE);	
	
	plist_free (proplistA);
	plist_free (proplistB);
}
END_TEST

START_TEST (test_set_get_unset)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    NValue *value = NULL;
    value = n_value_new ();
    n_value_set_int (value, 100);
    NValue *result = NULL;
    const char *key = "key";
    
    n_proplist_set (NULL, key, value);
    fail_unless (n_proplist_is_empty (proplist) == TRUE);
    n_proplist_set (proplist, NULL, value);
    fail_unless (n_proplist_is_empty (proplist) == TRUE);
    n_proplist_set (proplist, key, NULL);
    fail_unless (n_proplist_is_empty (proplist) == TRUE);
    n_proplist_set (proplist, key, value);
    fail_unless (n_proplist_size (proplist) == 1);

    result = n_proplist_get (NULL, key);
    fail_unless (result == NULL);
    result = n_proplist_get (proplist, NULL);
    fail_unless (result == NULL);
    result = n_proplist_get (proplist, key);
    fail_unless (result == value);
    fail_unless (n_value_equals (result, value) == TRUE);

    n_proplist_unset (NULL, key);
    fail_unless (n_proplist_size (proplist) == 1);
    n_proplist_unset (proplist, NULL);
    fail_unless (n_proplist_size (proplist) == 1);
    n_proplist_unset (proplist, key);
    fail_unless (n_proplist_is_empty (proplist) == TRUE);
    fail_unless (n_proplist_size (proplist) == 0);

    plist_free (proplist);
    n_value_free (value);
    value = NULL;
}
END_TEST


START_TEST (test_string)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    const char *key = "ngf";
    const char *value = "ValuE";
    
    n_proplist_set_string (NULL, key, value);
    fail_unless (n_proplist_size (proplist) == 0);
    n_proplist_set_string (proplist, NULL, value);
    fail_unless (n_proplist_size (proplist) == 0);
    n_proplist_set_string (proplist, key, NULL);
    fail_unless (n_proplist_size (proplist) == 0);
    
    n_proplist_set_string (proplist, key, value);
    fail_unless (n_proplist_size (proplist) == 1);
    const char *result = n_proplist_get_string (proplist, key);
    fail_unless (result != NULL);
    fail_unless (g_strcmp0 (result, value) == 0);
    fail_unless (n_proplist_get_string (NULL, key) == NULL);
    fail_unless (n_proplist_get_string (proplist, NULL) == NULL);

    char *duplicate = n_proplist_dup_string (proplist, key);
    fail_unless (duplicate != NULL);
    fail_unless (g_strcmp0 (duplicate, value) == 0);

    plist_free (proplist);
    g_free (duplicate);
}
END_TEST

START_TEST (test_int)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    const char *key = "ngf";
    int value = -100;

    n_proplist_set_int (NULL, key, value);
    fail_unless (n_proplist_size (proplist) == 0);
    n_proplist_set_int (proplist, NULL, value);
    fail_unless (n_proplist_size (proplist) == 0);
    
    n_proplist_set_int (proplist, key, value);
    fail_unless (n_proplist_size (proplist) == 1);
    int result = n_proplist_get_int (proplist, key);
    fail_unless (result == value);
    fail_unless (n_proplist_get_int (NULL, key) == 0);
    fail_unless (n_proplist_get_int (proplist, NULL) == 0);

    plist_free (proplist);
}
END_TEST

START_TEST (test_uint)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    const char *key = "ngf";
    uint value = 100;

    n_proplist_set_uint (NULL, key, value);
    fail_unless (n_proplist_size (proplist) == 0);
    n_proplist_set_uint (proplist, NULL, value);
    fail_unless (n_proplist_size (proplist) == 0);
    
    n_proplist_set_uint (proplist, key, value); 
    fail_unless (n_proplist_size (proplist) == 1);
    uint result = n_proplist_get_uint (proplist, key);
    fail_unless (result == value);
    fail_unless (n_proplist_get_uint (NULL, key) == 0);
    fail_unless (n_proplist_get_uint (proplist, NULL) == 0);

    plist_free (proplist);									
}
END_TEST

START_TEST (test_bool)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    const char *key = "ngf";
    gboolean value = TRUE;

    n_proplist_set_bool (NULL, key, value);
    fail_unless (n_proplist_size (proplist) == 0);
    n_proplist_set_bool (proplist, NULL, value);
    fail_unless (n_proplist_size (proplist) == 0);
    
    n_proplist_set_bool (proplist, key, value);
    fail_unless (n_proplist_size (proplist) == 1);
    gboolean result = n_proplist_get_bool (proplist, key);
    fail_unless (result == value);
    fail_unless (n_proplist_get_bool (NULL, key) == FALSE);
    fail_unless (n_proplist_get_bool (proplist, NULL) == FALSE);

    plist_free (proplist);
}
END_TEST 

START_TEST (test_pointer)
{
    NProplist *proplist = NULL;
    proplist = n_proplist_new ();
    fail_unless (proplist != NULL);
    const char *key = "ngf";
    gpointer value = proplist;
    
    n_proplist_set_pointer (NULL, key, value);
    fail_unless (n_proplist_size (proplist) == 0);
    n_proplist_set_pointer (proplist, NULL, value);
    fail_unless (n_proplist_size (proplist) == 0);
//    n_proplist_set_pointer (proplist, key, NULL);
//    fail_unless (n_proplist_size (proplist) == 0);
    
    n_proplist_set_pointer (proplist, key, value);
    fail_unless (n_proplist_size (proplist) == 1);
    gpointer result = n_proplist_get_pointer (proplist, key);
    fail_unless (result == value);
    fail_unless (n_proplist_get_pointer (NULL, key) == 0);
    fail_unless (n_proplist_get_pointer (proplist, NULL) == 0);

    plist_free (proplist);				     
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

	s = suite_create ("\tProplist tests");

	tc = tcase_create ("Create");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("copy");
	tcase_add_test (tc, test_copy);
	suite_add_tcase (s, tc);

	tc = tcase_create ("copy keys");
	tcase_add_test (tc, test_copy_keys);
	suite_add_tcase (s, tc);

	tc = tcase_create ("merge");
	tcase_add_test (tc, test_merge);
	suite_add_tcase (s, tc);

	tc = tcase_create ("merge keys");
	tcase_add_test (tc, test_merge_keys);
	suite_add_tcase (s, tc);

	tc = tcase_create ("size");
	tcase_add_test (tc, test_size);
	suite_add_tcase (s, tc);

	tc = tcase_create ("foreach");
	tcase_add_test (tc, test_foreach);
	suite_add_tcase (s, tc);

	tc = tcase_create ("is empty");
	tcase_add_test (tc, test_is_empty);
	suite_add_tcase (s, tc);

	tc = tcase_create ("has key");
	tcase_add_test (tc, test_has_key);
	suite_add_tcase (s, tc);

	tc = tcase_create ("match exact");
	tcase_add_test (tc, test_match_exact);
	suite_add_tcase (s, tc);

	tc = tcase_create ("set, get, unset value");
	tcase_add_test (tc, test_set_get_unset);
	suite_add_tcase (s, tc);

	tc = tcase_create ("string - get, set, duplicate");
	tcase_add_test (tc, test_string);
	suite_add_tcase (s, tc);

	tc = tcase_create ("int - get, set");
	tcase_add_test (tc, test_int);
	suite_add_tcase (s, tc);

	tc = tcase_create ("uint - get, set");
	tcase_add_test (tc, test_uint);
	suite_add_tcase (s, tc);

	tc = tcase_create ("bool - get, set");
	tcase_add_test (tc, test_bool);
	suite_add_tcase (s, tc);

	tc = tcase_create ("gpointer - get, set");
	tcase_add_test (tc, test_pointer);
	suite_add_tcase (s, tc);
	
	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
