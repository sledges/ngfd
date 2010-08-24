#include <stdlib.h>
#include <check.h>
#include <stdio.h>

#include "property.h"
#include "properties.h"
#include "state.c"

START_TEST (test_get_bool_string)
{
	GHashTable *htable = properties_new ();
	fail_unless (htable != NULL);

	Property *valBool = property_new ();
	fail_unless (valBool != NULL);
	const char *keyB = "Not";
	gboolean valB = TRUE;
	property_set_boolean (valBool, valB);
	g_hash_table_insert (htable, g_strdup (keyB), valBool);

	Property *valStr = property_new ();
	fail_unless (valStr != NULL);
	const char *keyS = "foo";
	const char *valS = "bar";
	property_set_string (valStr, valS);
	g_hash_table_insert (htable, g_strdup (keyS), valStr);

	// BOOL
	// use invalid key
	fail_unless (_properties_get_boolean (htable, "ngf") == FALSE);
	// use valid key for proper value
	fail_unless (_properties_get_boolean (htable, "Not") == TRUE);
	// use valid key for not boolean value
	fail_unless (_properties_get_boolean (htable, "foo") == FALSE); 

	// STRING
	// use invalid key
	fail_unless (_properties_get_string (htable, "ngf") == NULL);
	// use valid key for proper value
	fail_unless (strcmp (_properties_get_string (htable, "foo"), valS) == 0);
	// use valid key for not string value
	fail_unless (_properties_get_string (htable, "Not") == NULL);

	property_free (valBool);
	property_free (valStr);
	g_hash_table_destroy (htable);
	valBool = NULL;
	valStr = NULL;
	htable = NULL;
}
END_TEST

START_TEST (test_get_policy_play)
{
	GHashTable *htable = properties_new ();
	fail_unless (htable != NULL);

	Property *prop = property_new ();
	fail_unless (prop != NULL);
	gboolean propVal = TRUE;
	property_set_boolean (prop, propVal);

	// get policy.id - there is no such as key yet
	fail_unless (_properties_get_policy_id (htable) == 0);
	Property *valInt = property_new ();
	fail_unless (valInt !=  NULL);
	const char *keyI = "policy.id";
	gint valI = 1000;
	property_set_uint (valInt, valI);
	g_hash_table_insert (htable, g_strdup (keyI), prop);
	
	// get policy.id - boolean value instead of uint
	fail_unless (_properties_get_policy_id (htable) == 0);
	
	g_hash_table_remove (htable, keyI);
	g_hash_table_insert (htable, g_strdup (keyI), valInt);
	
	// get policy.id - proper part
	fail_unless (_properties_get_policy_id (htable) == valI);

	// get play.timeout - there is no such as key yet
	fail_unless (_properties_get_play_timeout (htable) == 0);
	
	const char* keyI2= "play.timeout";
	g_hash_table_insert (htable, g_strdup (keyI2), prop);
	
	// get play.timeout - string instead of uint
	fail_unless (_properties_get_play_timeout (htable) == 0);
	
	g_hash_table_remove (htable, keyI2);
	g_hash_table_insert (htable, g_strdup (keyI2), valInt);
	
	// get play.timeout - proper case
	fail_unless (_properties_get_play_timeout (htable) == valI);	

	property_free (prop);
	property_free (valInt);
	g_hash_table_destroy (htable);
	prop = NULL;
	valInt = NULL;
}
END_TEST

START_TEST (test_get_play_mode)
{
	GHashTable *htable = properties_new ();
	fail_unless (htable != NULL);
	Property *prop = property_new ();
	fail_unless (prop != NULL);
	const char *key = "play.mode";
	const char *modeShort = "short";
	const char *modeLong = "long";
	property_set_string (prop, modeShort);

	fail_unless (_properties_get_play_mode (htable) == 0);
	
	g_hash_table_insert (htable, g_strdup (key), prop);
	fail_unless (_properties_get_play_mode (htable) == REQUEST_PLAY_MODE_SHORT);

	g_hash_table_remove (htable, key);
	property_set_string (prop, modeLong);
	g_hash_table_insert (htable, g_strdup (key), prop);
	fail_unless (_properties_get_play_mode (htable) == REQUEST_PLAY_MODE_LONG);

	g_hash_table_remove (htable, key);
	property_set_string (prop, NULL);
	g_hash_table_insert (htable, g_strdup (key), prop);
	fail_unless (_properties_get_play_mode (htable) == 0);

	property_free (prop);
	g_hash_table_destroy (htable);
	prop = NULL;
	htable = NULL;
}
END_TEST

START_TEST (test_get_resources)
{
	GHashTable *htable = properties_new ();
	fail_unless (htable != NULL);

	Property *prop = property_new ();
	fail_unless (prop != NULL);
	
	int i;
	gboolean value = TRUE;
	Property *props[4];
	for (i = 0; i < 4; i++)
	{
		props[i] = property_new ();
		fail_unless (props[i] != NULL);
		property_set_boolean (props[i], value);
	}

	property_set_boolean (prop, value);
	const char* keys[4] = {	"media.audio\0",
				"media.vibra\0",
				"media.leds\0",
				"media.backlight\0"};
	
	fail_unless (_properties_get_resources (htable) == 0 );
	g_hash_table_insert (htable, g_strdup (keys[0]), prop);
	gint res = _properties_get_resources (htable);
	fail_unless (res == 1);
	g_hash_table_insert (htable, g_strdup (keys[1]), prop);	
	res = _properties_get_resources (htable);
	fail_unless (res == 3);
	g_hash_table_insert (htable, g_strdup (keys[2]), prop);
	res = _properties_get_resources (htable);
	fail_unless (res == 7);
	g_hash_table_insert (htable, g_strdup (keys[3]), prop);
	res = _properties_get_resources (htable);
	fail_unless (res == 15);

	for (i = 0; i < 4; i++)
	{
		property_free (props[i]);
		props[i] = NULL;
	}	
	*props = NULL;
	property_free (prop);
	g_hash_table_destroy (htable);
	prop = NULL;
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

	s = suite_create ("State");

	tc = tcase_create ("Get boolean, string");
	tcase_add_test (tc, test_get_bool_string);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get policy.id play.timeout");
	tcase_add_test (tc, test_get_policy_play);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get play.mode");
	tcase_add_test (tc, test_get_play_mode);
	suite_add_tcase (s, tc);

	tc = tcase_create ("Get resources");
	tcase_add_test (tc, test_get_resources);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;	
}
