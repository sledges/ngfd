#include <stdlib.h>
#include <check.h>
#include <stdio.h>

#include "property.h"
#include "properties.h"

START_TEST (test_create)
{
	GHashTable *htable = NULL;
	htable = properties_new ();

	fail_unless (htable != NULL);

	g_hash_table_destroy (htable);
	htable = NULL;
}
END_TEST

START_TEST (test_copy)
{
	GHashTable *source = properties_new ();
	fail_unless (source != NULL);

	Property *valStr = property_new ();
	fail_unless (valStr != NULL);
	const char *keyS = "foo";
	const char *valS = "bar";
	property_set_string (valStr, valS);

	Property *valInt = property_new ();
	fail_unless (valInt != NULL);
	const char *keyI = "AK";
	gint valI = 47;
	property_set_int (valInt, valI);

	Property *valBool = property_new ();
	fail_unless (valBool != NULL);
	const char *keyB = "Not";
	gboolean valB = TRUE;
	property_set_boolean (valBool, valB);

	g_hash_table_insert (source, g_strdup (keyS), valStr);
	g_hash_table_insert (source, g_strdup (keyI), valInt);
	g_hash_table_insert (source, g_strdup (keyB), valBool);

	GHashTable *copy = properties_copy (source);
	fail_unless (copy != NULL);
	fail_unless (g_hash_table_size (source) == g_hash_table_size (copy));
	const char *org = properties_get_string (source, "foo");
	const char *cp = properties_get_string (copy, "foo");
	fail_unless (strcmp (org, cp) ==0);
	fail_unless (properties_get_int (source, "AK") == properties_get_int (copy, "AK"));
	fail_unless (properties_get_bool (source, "Not") == properties_get_bool (copy, "Not"));

	property_free (valStr);
	property_free (valInt);
	property_free (valBool);
	g_hash_table_destroy (copy);
	g_hash_table_destroy (source);
	valStr = NULL;
	valInt = NULL;
	valBool = NULL;
	copy = NULL;
	source = NULL;
}
END_TEST 

static void list_table (GHashTable *);
	
START_TEST (test_merge)
{
	GHashTable *htsource = properties_new ();
	fail_unless (htsource != NULL);
		
	GHashTable *httarget = properties_new ();
	fail_unless (httarget != NULL);

        Property *valStr = property_new ();
	fail_unless (valStr != NULL);
	const char *keyS = "foo";
	const char *valS = "bar";
	property_set_string (valStr, valS);

	Property *valInt = property_new ();
	fail_unless (valInt != NULL);
	const char *keyI = "AK";
	gint valI = 47;
	property_set_int (valInt, valI);
	
	Property *valBool = property_new ();
	fail_unless (valBool != NULL);
	const char *keyB = "Not";
	gboolean valB = TRUE;
	property_set_boolean (valBool, valB);

	Property *copyInt = property_copy (valInt);
	gint newInt = 100000;
	property_set_int (copyInt, newInt);

	g_hash_table_insert (htsource, g_strdup (keyI), copyInt);	//this will replace valInt
	g_hash_table_insert (htsource, g_strdup (keyS), valStr);	//this will be added
	
	g_hash_table_insert (httarget, g_strdup (keyI), valInt);	//this change to copyInt
	g_hash_table_insert (httarget, g_strdup (keyB), valBool);	//this stay
	
	properties_merge (httarget, htsource);
	
	fail_unless (g_hash_table_size (httarget) == 3);
	fail_unless (strcmp (properties_get_string (httarget, keyS), valS) == 0);
	fail_unless (properties_get_int (httarget, keyI) == newInt);
	fail_unless (properties_get_bool (httarget, keyB) == valB);
	
	property_free (valStr);
	property_free (valInt);
	property_free (valBool);
	g_hash_table_destroy (htsource);
	g_hash_table_destroy (httarget);
	valStr = NULL;
	valInt = NULL;
	valBool = NULL;
	htsource = NULL;
	httarget = NULL;
}
END_TEST

static void
list_table (GHashTable *table)
{
	printf ("\nsize = %d\n", g_hash_table_size (table));
	GHashTableIter iter;
	const char *key = NULL;
	Property *value = NULL;
	g_hash_table_iter_init (&iter, table);
	while (g_hash_table_iter_next (&iter, (gpointer) &key, (gpointer) &value))
	{
		printf ("Key = %s\t", key);
		printf ("Value = %d\t",property_get_type (value));
		switch (property_get_type (value))
		{
			case 1:
				printf ("%s\n--------\n",property_get_string (value));
				break;
			case 3:
				printf ("%d\n--------\n",property_get_int (value));
				break;
			case 4:
				printf ("%s\n--------\n",(property_get_boolean (value))?"TRUE":"FALSE");
				break;
		}
			
	}
	value = NULL;
}

START_TEST (test_merge_allowed)
{
	GHashTable *htsource = properties_new ();
	fail_unless (htsource != NULL);
	
	GHashTable *httarget = properties_new (); 
	fail_unless (httarget != NULL);
	
	Property *valStr = property_new ();
	fail_unless (valStr != NULL);
	const char *keyS = "foo";
        const char *valS = "bar";
	property_set_string (valStr, valS);

	Property *valInt = property_new ();
	fail_unless (valInt != NULL);		
	const char *keyI = "AK";
	gint valI = 47;
	property_set_int (valInt, valI);

	Property *valBool = property_new ();
	fail_unless (valBool != NULL);
	const char *keyB = "Not";
	gboolean valB = TRUE;
	property_set_boolean (valBool, valB); 
		
	Property *valInt1 = property_new ();
	fail_unless (valInt1 != NULL);
	const char *keyI1 = "Route";
	gint valI1 = 66;
	property_set_int (valInt1, valI1);
	
	Property *valStr1 = property_new ();
	fail_unless (valStr1 != NULL);
	const char *keyS1 = "Right";
	const char *valS1 = "Now";
	property_set_string (valStr1, valS1);
	
	Property *copyStr = property_copy (valStr);	// copy of foo/bar
	gboolean newBool = FALSE;
	property_set_boolean (copyStr, newBool); 	// change to BOOLEAN - foo/FALSE
	
	g_hash_table_insert (htsource, g_strdup (keyB), valBool);
	g_hash_table_insert (htsource, g_strdup (keyS), valStr); 
	g_hash_table_insert (htsource, g_strdup (keyI1), valInt1);
	g_hash_table_insert (htsource, g_strdup (keyS1), valStr1);
	g_hash_table_insert (httarget, g_strdup (keyI), valInt);
	g_hash_table_insert (httarget, g_strdup (keyS), copyStr);

	char *allowed[3];
	allowed[0] = NULL;
	allowed[1] = NULL;
	allowed[2] = NULL;

	guint size_before = g_hash_table_size (httarget);
	properties_merge_allowed (httarget, htsource, allowed);	// nothing should happen
	guint size_after = g_hash_table_size (httarget);
	fail_unless (size_before == size_after);

	allowed[0] = g_strdup (keyB);	// allow key Not -> will add key Not with value TRUE
	allowed[1] = g_strdup (keyS);	// allow key foo -> will replace BOOLEAN foo/FALSE to foo/bar
	allowed[2] = g_strdup (NULL);	// will do nothing anyway

	// So two entries in target table, one will be replaced and one added.
	// Result - three entries in target table.
	
	properties_merge_allowed (httarget, htsource, allowed);
	size_after = g_hash_table_size (httarget);
	fail_unless (size_after == 3);
	
	g_free (allowed[0]);
	g_free (allowed[1]);
	property_free (valStr);
	property_free (valInt);
	property_free (valBool);
	property_free (copyStr);
	property_free (valStr1);
	property_free (valInt1);
	g_hash_table_destroy (htsource);
	g_hash_table_destroy (httarget);
	valStr = NULL;
	valInt = NULL;
	valBool = NULL;
	copyStr = NULL;
	valStr1 = NULL;
	valInt1 = NULL;
	htsource = NULL;
	httarget = NULL;
}
END_TEST

START_TEST (test_get_string)
{
	GHashTable *htable = properties_new ();
	fail_unless (htable != NULL);
	Property *value = property_new ();
	fail_unless (value != NULL);
	const char *key = "foo";
	const char *val = "bar";
	property_set_string (value, val);

	g_hash_table_insert (htable, g_strdup (key), value);
	const char* returned = properties_get_string (htable, "foobar");
	fail_unless (returned == NULL);

	guint retInt = properties_get_int (htable, "foo");
	fail_unless (retInt == -1);

	gboolean retBool = properties_get_bool (htable, "foo");
	fail_unless (retBool == FALSE);
	
	returned = properties_get_string (htable, "foo");
	fail_unless (strcmp (returned, val) == 0);

	property_free (value);
	g_hash_table_destroy (htable);
	value = NULL;
	htable = NULL;
}
END_TEST

START_TEST (test_get_int)
{
	GHashTable *htable = properties_new ();
	fail_unless (htable != NULL);
	Property *value = property_new ();
	fail_unless (value != NULL);
	const char *key = "AK";
	gint val = 47;
	property_set_int (value, val);
	
	g_hash_table_insert (htable, g_strdup (key), value);
	gint returned = properties_get_int (htable, "AK47");
	fail_unless (returned == -1);

	gboolean retBool = properties_get_bool (htable, "AK");
	fail_unless (retBool == FALSE);

	const char *retStr = properties_get_string (htable, "AK");
	fail_unless (retStr == NULL);

	returned = properties_get_int (htable, "AK");
	fail_unless (returned == val);

	property_free (value);
	g_hash_table_destroy (htable);
	value = NULL;
	htable = NULL;
}
END_TEST

START_TEST (test_get_bool)
{
	GHashTable *htable = properties_new ();
	fail_unless (htable != NULL);
	Property *value = property_new ();
	fail_unless (value != NULL);
	const char *key = "Not";
	gboolean val = TRUE;
	property_set_boolean (value, val);

	g_hash_table_insert (htable, g_strdup (key), value);
	gboolean returned = properties_get_bool (htable, "NotTRUE");
	fail_unless (returned == FALSE);

	const char *retStr = properties_get_string (htable, "Not");
	fail_unless (retStr == NULL);

	gint retInt = properties_get_int (htable, "Not");
	fail_unless (retInt == -1);

	returned = properties_get_bool (htable, "Not");
	fail_unless (returned == TRUE);
	
	property_free (value);
	g_hash_table_destroy (htable);
	value = NULL;
	htable = NULL;
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
	
	s = suite_create ("Properties");
	
	tc = tcase_create ("Create properties");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Copy table");
	tcase_add_test (tc, test_copy);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Merge tables");
	tcase_add_test (tc, test_merge);
	suite_add_tcase (s, tc);
	
	tc = tcase_create ("Merge allowed");
	tcase_add_test (tc, test_merge_allowed);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get string");
	tcase_add_test (tc, test_get_string);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get integer");
	tcase_add_test (tc, test_get_int);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get boolean");
	tcase_add_test (tc, test_get_bool);
	suite_add_tcase (s, tc);
	
	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	
	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
