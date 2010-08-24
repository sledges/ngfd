#include <stdlib.h>
#include <check.h>
#include <glib.h>
#include <string.h>
#include <stdio.h>

#include "property.h"

START_TEST (test_create)
{
	Property *property = NULL;
	property = property_new ();

	fail_unless (property != NULL);
	property_free (property);
	property = NULL;
}
END_TEST

START_TEST (test_getType)
{
	Property *property = property_new ();
	fail_unless (property != NULL);
	property->type = PROPERTY_TYPE_UINT;
	guint type = property_get_type (property);

	fail_unless (type == PROPERTY_TYPE_UINT);
	property_free (property);
	property = NULL;
}
END_TEST

START_TEST (test_string)
{
	Property *property = property_new();
	fail_unless (property != NULL);
	const char *str = "STRING";
	property_set_string (property, str);

	fail_unless (property_get_type (property) == PROPERTY_TYPE_STRING);
	fail_unless (strcmp (property->value.s, str) == 0);
	
	const char *returned = property_get_string (property);
	fail_unless (strcmp (returned, str) == 0);

	property_free (property);
	property = NULL;
}
END_TEST

START_TEST (test_guint)
{
	Property *property = property_new ();
	fail_unless (property != NULL);
	guint var = 100;
	property_set_uint (property, var);

	fail_unless (property_get_type (property) == PROPERTY_TYPE_UINT);
	fail_unless (property->value.u == var);

	guint returned = property_get_uint (property);
	fail_unless (returned == var);

	property_free (property);
	property = NULL;
}
END_TEST

START_TEST (test_gint)
{
	Property *property = property_new ();
	fail_unless (property != NULL);
	gint var = 100;
	property_set_int (property, var);

	fail_unless (property_get_type (property) == PROPERTY_TYPE_INT);
	fail_unless (property->value.i == var);

	gint returned = property_get_int (property);
	fail_unless (returned == var);

	property_free (property);
	property = NULL;
}
END_TEST

START_TEST (test_boolean)
{
	Property *property = property_new ();
	fail_unless (property != NULL);
	gboolean var = TRUE;
	property_set_boolean (property, var);

	fail_unless (property_get_type (property) == PROPERTY_TYPE_BOOLEAN);
	fail_unless (property->value.b == var);

	gboolean returned = property_get_boolean (property);
	fail_unless (returned == var);

	property_free (property);
	property = NULL;
	
}
END_TEST

START_TEST (test_copy)
{
	Property *property = NULL;
	property = property_new ();
	fail_unless (property != NULL);
	const char *str = "STRING";
	property_set_string (property, str);
	Property* copy = property_copy (property);
	
	fail_unless (property_get_type (copy) == PROPERTY_TYPE_STRING);
	fail_unless (strcmp (property_get_string (copy), str) == 0);
	
	property_free (copy);
	property_free (property);
	copy = NULL;
	property = NULL;
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
	
	s = suite_create ("Property");
	
	tc = tcase_create ("Create property");
	tcase_add_test (tc, test_create);
	suite_add_tcase (s, tc);
	
        tc = tcase_create ("Get type");
        tcase_add_test (tc, test_getType);
        suite_add_tcase (s, tc);

        tc = tcase_create ("Get and Set string");
        tcase_add_test (tc, test_string);
        suite_add_tcase (s, tc);

	tc = tcase_create ("Get and Set uint");
	tcase_add_test (tc, test_guint);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get and Set int");
	tcase_add_test (tc, test_gint);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get and Set boolean");
	tcase_add_test (tc, test_boolean);
	suite_add_tcase (s, tc);
	
        tc = tcase_create ("Copy property");
        tcase_add_test (tc, test_copy);
        suite_add_tcase (s, tc);
	
	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	
	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
