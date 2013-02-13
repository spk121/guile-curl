#define _GNU_SOURCE
#include <config.h>
#include <assert.h>
#include <stdio.h>
#include <libguile.h>
#include <curl/curl.h>
#include "type.h"

#define SCM_C_LIST_REF(x,i) scm_list_ref(x,scm_from_size_t(i))
#define SCM_C_LIST_LENGTH(x) scm_to_size_t(scm_length(x))
#define SCM_IS_LIST(x) scm_is_true(scm_list_p(x))

static scm_t_bits handle_tag;

SCM equalp_handle (SCM x1, SCM x2);
size_t gc_free_handle (SCM x);
int print_handle (SCM x, SCM port, scm_print_state *pstate);


/* handle -- In C, a handle_post_t pointer.  In scheme, a struct that
   contains that pointer. */

int
_scm_is_handle (SCM x)
{
  if (SCM_SMOB_PREDICATE (handle_tag, x))
    {
      if ((handle_post_t *) SCM_SMOB_DATA (x) != (handle_post_t *) NULL)
	return 1;
      else
	return 0;
    }
  else
    return 0;
}

handle_post_t *
_scm_to_handle (SCM x)
{
  return (handle_post_t *) SCM_SMOB_DATA (x);
}

SCM
_scm_from_handle (handle_post_t *x)
{
  SCM s_handle;

  SCM_CRITICAL_SECTION_START;

  assert (x != NULL);

  SCM_NEWSMOB (s_handle, handle_tag, x);

  assert (x == (handle_post_t *) SCM_SMOB_DATA (s_handle));

  if (0)
    {
      fprintf (stderr, "Making <#handle> smob from CURL * %p\n", x);
    }
  SCM_CRITICAL_SECTION_END;
  return (s_handle);
}

// Handles are equal if they point to the same C structure
SCM
equalp_handle (SCM x1, SCM x2)
{
  SCM ret;
  CURL *handle1;
  CURL *handle2;

  SCM_CRITICAL_SECTION_START;
  handle1 = (handle_post_t *) SCM_SMOB_DATA (x1);
  handle2 = (handle_post_t *) SCM_SMOB_DATA (x2);

  if ((handle1 == NULL) || (handle2 == NULL))
    ret = SCM_BOOL_F;
  else if ((handle1 != handle2))
    ret = SCM_BOOL_F;
  else
    ret = SCM_BOOL_T;
  SCM_CRITICAL_SECTION_END;
  return ret;
}

size_t
gc_free_handle (SCM handle)
{
  SCM_ASSERT (SCM_SMOB_PREDICATE (handle_tag, handle), handle, SCM_ARG1, "free-handle");

  SCM_CRITICAL_SECTION_START;

  handle_post_t *m = _scm_to_handle (handle);

  assert (m != NULL);

  if (m->handle != NULL)
    {
      curl_easy_cleanup (m->handle);
      m->handle = NULL;
    }
  if (m->postfields != NULL)
    {
      free (m->postfields);
      m->postfields = NULL;
    }
  if (m->httppost != NULL)
    {
      curl_formfree (m->httppost);
      m->httppost = NULL;
    }
  if (m->httpheader != NULL)
    {
      curl_slist_free_all (m->httpheader);
      m->httpheader = NULL;
    }
  if (m->http200aliases != NULL)
    {
      curl_slist_free_all (m->http200aliases);
      m->http200aliases = NULL;
    }
  if (m->mail_rcpt != NULL)
    {
      curl_slist_free_all (m->mail_rcpt);
      m->mail_rcpt = NULL;
    }
  if (m->quote != NULL)
    {
      curl_slist_free_all (m->quote);
      m->quote = NULL;
    }
  if (m->postquote != NULL)
    {
      curl_slist_free_all (m->postquote);
      m->postquote = NULL;
    }
  if (m->prequote != NULL)
    {
      curl_slist_free_all (m->prequote);
      m->prequote = NULL;
    }
  if (m->resolve != NULL)
    {
      curl_slist_free_all (m->resolve);
      m->resolve = NULL;
    }
  if (m->telnetoptions != NULL)
    {
      curl_slist_free_all (m->telnetoptions);
      m->telnetoptions = NULL;
    }
  free (m);
  m = NULL;
  SCM_CRITICAL_SECTION_END;

  return 0;
}

int
#ifdef __GNUC__
print_handle (SCM x, SCM port, scm_print_state *pstate __attribute__ ((unused)))
#else
  print_handle (SCM x, SCM port, scm_print_state *pstate)
#endif
{
  handle_post_t *frm = (handle_post_t *) SCM_SMOB_DATA (x);
  char *str;

  scm_puts ("#<handle ", port);

  if (frm == (handle_post_t *) NULL)
    {
      scm_puts ("(freed)", port);
    }
  else
    {
      if (asprintf (&str, "%p", frm) < 0)
	scm_puts ("???", port);
      else
	scm_puts (str, port);
    }

  scm_puts (">", port);

  // non-zero means success
  return 1;
}

SCM
cl_is_handle_p (SCM x)
{
  return scm_from_bool (_scm_is_handle (x));
}

/* slist -- In C, a struct curl_slist *.
   In Scheme, a list of strings of bytevectors */

int
_scm_can_convert_to_slist (SCM x)
{
  size_t i, n;
  SCM elt;

  if (!SCM_IS_LIST(x))
    return 0;

  n = SCM_C_LIST_LENGTH(x);
  if (n == 0)
    return 1;			/* Empty slist */
  for (i = 0; i < n; i ++)
    {
      elt = SCM_C_LIST_REF (x, i);
      if (!scm_is_string (elt))
	return 0;
    }
  return 1;
}

struct curl_slist *
_scm_convert_to_slist (SCM x)
{
  int i, n;
  SCM elt;
  char *str;
  struct curl_slist *slist = NULL;

  n = scm_to_int (scm_length (x));
  if (n == 0)
    return NULL;
  for (i = 0; i < n; i ++)
    {
      elt = SCM_C_LIST_REF(x, i);
      if (scm_is_string (elt))
	str = scm_to_locale_string (elt);
      slist = curl_slist_append (slist, str);
    }
  return slist;
}

/* For Guile-1.8, tests to see if this is a string.  For Guile-2.0,
   tests to see if this is a string or bytevector.  */

#define SCM_C_STRING_REF(s,i) \
  scm_to_int (scm_char_to_integer (scm_c_string_ref (s, i)))

int
_scm_can_convert_to_byte_data (SCM x)
{
#if GUILE_MAJOR_VERSION == 1
  if (scm_is_string (x))
    return 1;
  else
    return 0;
#else
  {
    int i, n;
    int c;

    if (scm_is_bytevector (x))
      return 1;
    else if (scm_is_string (x))
      {
	n = scm_c_string_length (x);
	if (n == 0)
	  return 1;		/* Empty string */
	for (i = 0; i < n; i ++)
	  {
	    c = SCM_C_STRING_REF (x, i);
	    if (c > 255)
	      return 0;
	  }
	return 1;
      }
  }
    /* else */
    return 0;
#endif
}

/* For Guile-1.8, returns the memory location of the chars of a
   string.  For Guile-2.0, returns the memory location of the bytes of
   a bytevector. Since these are SCM objects, they are GC'd.  */
uint8_t *
_scm_convert_to_byte_data (SCM x, size_t *len)
{
#if GUILE_MAJOR_VERSION == 1
  return (uint8_t *) scm_to_locale_stringn (x, len);
#else
  if (scm_is_string (x))
    {
      size_t i;
      SCM bv;

      *len = scm_c_string_length (x);
      if (*len == 0)
	return NULL;

      bv = scm_c_make_bytevector (*len);
      for (i = 0; i < *len; i ++)
	{
	  SCM_BYTEVECTOR_CONTENTS(bv)[i] = SCM_C_STRING_REF (x, i);
	}
      return (uint8_t *) SCM_BYTEVECTOR_CONTENTS (bv);
    }
  else
    {
      *len = SCM_BYTEVECTOR_LENGTH (x);
      return SCM_BYTEVECTOR_CONTENTS (x);
    }
#endif
}


/* Each entry in an http post is a list of 2 to 4 elements.
   1. name - string
   2. data - byte data
   3. mime - string, optional
   4. filename - string, optional
*/
static int
_scm_can_convert_to_httppost_entry (SCM x)
{
  size_t n;

  if (!SCM_IS_LIST(x))
    return 0;
  n = SCM_C_LIST_LENGTH(x);
  if (n < 2 || n > 4)
    return 0;
  if (!scm_is_string (SCM_C_LIST_REF(x, 0)))
    return 0;
  if (!_scm_can_convert_to_byte_data (SCM_C_LIST_REF(x, 1)))
    return 0;
  if (n == 3 && !scm_is_string (SCM_C_LIST_REF(x, 2)))
    return 0;
  if (n == 4 && !scm_is_string (SCM_C_LIST_REF(x, 3)))
    return 0;

  return 1;
}

int
_scm_can_convert_to_httppost (SCM x)
{
  int i, n;
  SCM elt;

  if (!SCM_IS_LIST(x))
    return 0;

  n = SCM_C_LIST_LENGTH(x);
  if (n == 0)
    return 1;			/* Empty post */
  for (i = 0; i < n; i ++)
    {
      elt = SCM_C_LIST_REF(x, i);
      if (!_scm_can_convert_to_httppost_entry (elt))
	return 0;
    }
  return 1;
}

struct curl_httppost *
_scm_convert_to_httppost (SCM x)
{
  struct curl_httppost *post = NULL;
  struct curl_httppost *last = NULL;
  size_t j,m,n;

  n = SCM_C_LIST_LENGTH (x);
  for (j = 0; j < n; j ++)
    {
      char *name = NULL, *mime = NULL, *filename = NULL;
      uint8_t *contents = NULL;
      size_t name_len, mime_len, filename_len, contents_len;

      SCM elt = SCM_C_LIST_REF (x, j);
      m = SCM_C_LIST_LENGTH (elt);

      name = scm_to_locale_stringn (SCM_C_LIST_REF (elt, 0), &name_len);
      contents = _scm_convert_to_byte_data (SCM_C_LIST_REF (elt, 1),
					    &contents_len);
      if (m >= 3)
	mime = scm_to_locale_stringn (SCM_C_LIST_REF (elt, 2), &mime_len);
      if (m == 4)
	filename = scm_to_locale_stringn (SCM_C_LIST_REF (elt, 3), &filename_len);
      if (m == 2)
	curl_formadd (&post, &last,
		      CURLFORM_PTRNAME, name,
		      CURLFORM_NAMELENGTH, name_len,
		      CURLFORM_PTRCONTENTS, contents,
		      CURLFORM_CONTENTSLENGTH, contents_len,
		      CURLFORM_END);
      else if (m == 3)
	{
	  curl_formadd (&post, &last,
			CURLFORM_PTRNAME, name,
			CURLFORM_NAMELENGTH, name_len,
			CURLFORM_PTRCONTENTS, contents,
			CURLFORM_CONTENTSLENGTH, contents_len,
			CURLFORM_CONTENTTYPE, mime,
			CURLFORM_END);
	  free (mime);
	}
      else if (m == 4)
	{
	  curl_formadd (&post, &last,
			CURLFORM_PTRNAME, name,
			CURLFORM_NAMELENGTH, name_len,
			CURLFORM_PTRCONTENTS, contents,
			CURLFORM_CONTENTSLENGTH, contents_len,
			CURLFORM_CONTENTTYPE, mime,
			CURLFORM_FILENAME, filename,
			CURLFORM_END);
	  free (mime);
	  free (filename);
	}
    }
  return post;
}

void
cl_init_type ()
{
  handle_tag = scm_make_smob_type ("handle", sizeof (CURL *));
  scm_set_smob_free (handle_tag, gc_free_handle);
  scm_set_smob_print (handle_tag, print_handle);
  scm_set_smob_equalp (handle_tag, equalp_handle);
  scm_c_define_gsubr ("handle?", 1, 0, 0, cl_is_handle_p);
}
