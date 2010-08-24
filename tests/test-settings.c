#include <stdlib.h>
#include <check.h>
#include <stdio.h>

#include "context.h"
#include "definition.h"
#include "settings.c"

START_TEST (test_string_operation)
{
	//	TYPE
	char *type = _strip_group_type ("event name@parent\0");
	// correct case
	fail_unless (strcmp (type, "name@parent") == 0);
	
	fail_unless (_strip_group_type ("a \0") == NULL);
	g_free (type);
	type = NULL;

	
	//	NAME
	char *name = _parse_group_name ("event name@parent\0");
	// correct case
	fail_unless (strcmp (name, "name") == 0);

	fail_unless (_parse_group_name ("a \0") == NULL);
	// linia 123 pozostala - korekta ?
	g_free (name);
	name = NULL;


	//	PARENT
	char *parent = _parse_group_parent ("event name@parent\0");
	// correct case
	fail_unless (strcmp (parent, "parent") == 0);
	
	fail_unless (_parse_group_parent (NULL) == NULL);
	fail_unless (_parse_group_parent ("a \0") == NULL);
	fail_unless (_parse_group_parent ("event nameparent\0") == NULL);

	g_free (parent);
	parent = NULL;	
}
END_TEST

START_TEST (test_parse_general)
{
	Context *context = NULL;
	fail_unless ((context = g_new0 (Context, 1)) != NULL);

	SettingsData *data = g_new0 (SettingsData, 1);
	data->context = context;

	GKeyFile *k = NULL;
	fail_unless ((k = g_key_file_new ()) != NULL);
	const char *vibra = "VIBRA";
	int buffer = 1000;
	int latency = 1;
	g_key_file_set_string (k, GROUP_GENERAL, "vibration_search_path", vibra);
	g_key_file_set_integer (k, GROUP_GENERAL, "buffer_time", buffer);
	g_key_file_set_integer (k, GROUP_GENERAL, "latency_time", latency);
	
	_parse_general (data, k);

	fail_unless (strcmp (context->patterns_path, vibra) == 0);
	fail_unless (context->audio_buffer_time == buffer);
	fail_unless (context->audio_latency_time == latency);

	g_free (data);
	g_free (context);
	data = NULL;
	context = NULL;
}
END_TEST

START_TEST (test_parse_definitions)
{
	Context *context = NULL;
	fail_unless ((context = g_new0 (Context, 1)) != NULL);

	context->definitions = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) definition_free);
	fail_unless (context->definitions != NULL);

	SettingsData *data = g_new0 (SettingsData, 1);
	data->context = context;

	GKeyFile *k = NULL;
	fail_unless ((k = g_key_file_new ()) != NULL);
	const gchar gr[6] = "group\0";
	gchar group[20];
	sprintf (group, "%s %s", GROUP_DEFINITION, gr);
	g_key_file_set_string (k, group, "long", "ring");
	g_key_file_set_string (k, group, "short", "beep");
	g_key_file_set_string (k, group, "meeting", "meet");

	_parse_definitions (data, k);

	GList *list = g_hash_table_get_keys (context->definitions);
	fail_unless (strcmp ((const char*) list->data, gr) == 0);
	
	gpointer result = g_hash_table_lookup (context->definitions, gr);
	fail_unless (strcmp (((Definition*) result)->long_event, "ring") == 0);
	fail_unless (strcmp (((Definition*) result)->short_event, "beep") == 0);
	fail_unless (strcmp (((Definition*) result)->meeting_event, "meet") == 0);
	
	g_list_free (result);
	g_list_free (list);
	g_free (data);
	g_free (context);
	result = NULL;
	list = NULL;
	data = NULL;
	context = NULL;
}
END_TEST

START_TEST (test_event_is_done)
{
	gboolean result = TRUE;
	result = _event_is_done (NULL, NULL);
	fail_unless (result == FALSE);

	char *data = "ngf";
	GList *list = NULL;
	list = g_list_append (list, (gpointer) data);
	result = _event_is_done (list, data);
	fail_unless (result == TRUE);
	result = _event_is_done (list, "today");
	fail_unless (result == FALSE);

	g_list_free (list);
	list = NULL;
}
END_TEST

START_TEST (test_add_property_int)
{
	GHashTable *target =  g_hash_table_new_full (g_str_hash, g_str_equal, g_free, (GDestroyNotify) property_free);
	GKeyFile *k = NULL;
	fail_unless ((k = g_key_file_new ()) != NULL);
	const char *group = "group";
	const char *key = "route";
	g_key_file_set_integer (k, group, key, 66);
	
	guint size1 = g_hash_table_size (target);
	_add_property_int (target, k, group, "ngf", 0, FALSE);
	guint size2 = g_hash_table_size (target);
	fail_unless ( size2 == size1 );
	
	_add_property_int (target, k, group, key, 0, FALSE);
	size2 = g_hash_table_size (target);
	fail_unless (size1 == size2 - 1);

	g_hash_table_destroy (target);
	g_key_file_free (k);
	target = NULL;
	k = NULL;
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

	s = suite_create ("Settings");

	tc = tcase_create ("test splitting settings");
	tcase_add_test (tc, test_string_operation);
	suite_add_tcase (s, tc);

	tc = tcase_create ("test parse general");
	tcase_add_test (tc, test_parse_general);
	suite_add_tcase (s, tc);

	tc = tcase_create ("test parse definitions");
	tcase_add_test (tc, test_parse_definitions);
	suite_add_tcase (s, tc);

	tc = tcase_create ("test event is done");
	tcase_add_test (tc, test_event_is_done);
	suite_add_tcase (s, tc);

	tc = tcase_create ("test add property int");
	tcase_add_test (tc, test_add_property_int);
	suite_add_tcase (s, tc);

	sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	num_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
