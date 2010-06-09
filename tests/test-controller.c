#include <stdlib.h>
#include <string.h>
#include <check.h>
#include <glib.h>

#include "controller.h"

typedef struct _StepData
{
    guint step_time;
    guint step_value;
} StepData;

static const StepData step_data[] = {
    { 0, 0 },
    { 1000, 1 },
    { 2000, 2 },
    { 3000, 3 },
    { 4000, 4 },
};

#define ARRAY_SIZE(array) (sizeof (array) / sizeof (array[0]))

START_TEST (test_add_steps)
{
    Controller *controller = NULL;
    ControllerStep *step = NULL;
    GList *steps = NULL, *iter = NULL;
    int i;

    controller = controller_new ();
    fail_unless (controller != NULL);

    steps = controller_get_steps (controller);
    fail_unless (steps == NULL);

    /* Verify that added steps are in order */

    for (i = 0; i < ARRAY_SIZE (step_data); i++)
        controller_add_step (controller, step_data[i].step_time, step_data[i].step_value, FALSE);

    steps = controller_get_steps (controller);
    fail_unless (steps != NULL);

    for (iter = g_list_first (steps), i = 0; iter; iter = g_list_next (iter), i++) {
        step = (ControllerStep*) iter->data;

        fail_unless (step->time == step_data[i].step_time);
        fail_unless (step->value == step_data[i].step_value);
    }

    controller_free (controller);
    controller = NULL;
}
END_TEST

static gboolean test_execute_steps_loop_started = FALSE;
static gint test_execute_steps_counter = 0;

static void
_controller_cb (Controller *controller, guint step_time, guint step_value, gboolean is_last, gpointer userdata)
{
    gint counter = test_execute_steps_counter++;

    fail_unless (GPOINTER_TO_INT (userdata) == 0xB000BEEF);
    fail_unless (step_time == step_data[counter].step_time);
    fail_unless (step_value == step_data[counter].step_value);

    if (counter == 0)
        fail_unless (test_execute_steps_loop_started == FALSE);
    else
        fail_unless (test_execute_steps_loop_started == TRUE);
}

START_TEST (test_execute_steps)
{
    Controller *controller = NULL;
    guint id;
    int i;

    controller = controller_new ();
    fail_unless (controller != NULL);

    for (i = 0; i < ARRAY_SIZE (step_data); i++)
        controller_add_step (controller, step_data[i].step_time, step_data[i].step_value, FALSE);

    /* Verify that the first step is executed immediately. */
    id = controller_start (controller, _controller_cb, (gpointer) 0xB000BEEF);
    fail_unless (id > 0);

    controller_free (controller);
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

    s = suite_create ("Controller");

    tc = tcase_create ("Adding and verifying steps");
    tcase_add_test (tc, test_add_steps);
    suite_add_tcase (s, tc);

    tc = tcase_create ("Execute steps and verify values");
    tcase_add_test (tc, test_execute_steps);
    suite_add_tcase (s, tc);

    sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    num_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return num_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
