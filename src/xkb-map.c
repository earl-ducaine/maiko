
#include <gmodule.h>
#include <glib-object.h>

GHashTable *hash1;  

typedef struct _MyBoxed MyBoxed;

struct _MyBoxed
{
  gint ivalue;
  gchar *bla;
};

static gpointer
my_boxed_copy (gpointer orig)
{
  MyBoxed *a = orig;
  MyBoxed *b;

  b = g_slice_new (MyBoxed);
  b->ivalue = a->ivalue;
  b->bla = g_strdup (a->bla);

  return b;
}

static gint my_boxed_free_count;

static void
my_boxed_free (gpointer orig)
{
  MyBoxed *a = orig;

  g_free (a->bla);
  g_slice_free (MyBoxed, a);

  my_boxed_free_count++;
}


static void
test_boxed_hashtable (void)
{
  GHashTable *v;
  GHashTable *v2;
  GValue value = G_VALUE_INIT;

  g_value_init (&value, G_TYPE_HASH_TABLE);
  g_assert (G_VALUE_HOLDS_BOXED (&value));

  v = g_hash_table_new (g_str_hash, g_str_equal);
  g_value_take_boxed (&value, v);

  v2 = g_value_get_boxed (&value);
  g_assert (v2 == v);

  v2 = g_value_dup_boxed (&value);
  g_assert (v2 == v);  /* hash tables use ref/unref for copy/free */
  g_hash_table_unref (v2);

  g_value_unset (&value);
}


static void
test_boxed_keyfile (void)
{
  GKeyFile *k, *k2;
  GValue value = G_VALUE_INIT;

  g_value_init (&value, G_TYPE_KEY_FILE);
  g_assert (G_VALUE_HOLDS_BOXED (&value));

  k = g_key_file_new ();
  g_value_take_boxed (&value, k);

  k2 = g_value_get_boxed (&value);
  g_assert (k == k2);

  k2 = g_value_dup_boxed (&value);
  g_assert (k == k2);  /* keyfile uses ref/unref for copy/free */
  g_key_file_unref (k2);

  g_value_unset (&value);
}
