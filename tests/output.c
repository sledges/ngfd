#include <stdio.h>
#include <stdlib.h>

typedef enum
{
	INT,
	STR
} MessageType;

void
debug (gpointer message, MessageType t)
{
	if (t == INT)
	{
		int msg = (gint)message;
		printf ("\n%d\n\n", msg);
	}
	else if (t == STR)
	{
		char *msg = (gchar*)message;
		printf ("\n%s\n\n", msg);
	}
	else
	{
		printf ("No such a type of data.\n");
	}
}

char **
keys_init (int n)
{
    char **keys;
    keys = g_new0 (char*, n);
    int i = 0;
    for (i = 0; i < n; i++)
    {
        keys[i] = g_strdup_printf ("key%d", i);		
    }
    return keys;
}

char **
keys_init_custom (int n, char *pattern)
{
    char **keys;
    keys = g_new0 (char*, n);
    int i = 0;
    for (i = 0; i < n; i++)
    {
        keys[i] = g_strdup_printf ("%s%d", pattern, i);
    }
    return keys;
}

int *
int_values (int n)
{
    int *intValues;
	intValues = g_new0 (int, n);
    int i = 0;
	for (i = 0; i < n; i++)
	{
		intValues[i] = -10 * i + 11;
	}
	return intValues;
}

uint *
uint_values (int n)
{
    uint *uintValues;
	uintValues = g_new0 (uint, n);
    int i = 0;
	for (i = 0; i < n; i++)
	{
		uintValues[i] = 10 * i + 15;
	}
	return uintValues;
}

char**
str_values (int n)
{
    char **strValues;
	strValues = g_new0 (char*, n);
    int i = 0;
	for (i = 0; i < n; i++)
	{
		strValues[i] = g_strdup_printf ("value%d", i);
	}
	return strValues;
}

gboolean *
bool_values (int n)
{
    gboolean *boolValues;
	boolValues = g_new0 (gboolean, n);
    int i = 0;
	for (i = 0; i < n; i++)
	{
		boolValues[i] = i % 2;
	}
	return boolValues;
}

void
keys_free (char **keys, int n)
{
    if (!keys)
        return;
    int i = 0;
    for (i = 0; i < n; i++)
    {
        g_free (keys[i]);
    }
    g_free (keys);
}

void
keys_print (char **keys, int n)
{
    if (!keys)
        return;
    int i = 0;
    for (i = 0; i < n; i++)
    {
        printf ("%s\n", keys[i]);
    }
}

void
int_values_free (int *values)
{
    if (!values)
        return;
    g_free (values);
}

void
int_values_print (int *values, int n)
{
    if (!values)
        return;
    int i = 0;
    for (i = 0; i < n; i++)
    {
        printf ("%d\n", values[i]);
    }
}

void
uint_values_free (uint *values)
{
    if (!values)
        return;
    g_free (values);
}

void
uint_values_print (int *values, int n)
{
    if (!values)
        return;
    int i = 0;
    for (i = 0; i < n; i++)
    {
        printf ("%u\n", values[i]);
    }
}

void
str_values_free (char **values, int n)
{
    keys_free (values, n);
}

void 
str_values_print (char **values, int n)
{
    keys_print (values, n);
}

void
bool_values_free (gboolean *values)
{
    if (!values)
        return;
    g_free (values);
}

void
bool_values_print (gboolean *values, int n)
{
    if (!values)
        return;
    int i = 0;
    for (i = 0; i < n; i++)
    {
        printf ("%s\n", values[i] == TRUE ? "TRUE" : "FALSE");
    }
}
