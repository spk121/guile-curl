/* Type helper functions for guile-curl

   Copyright (c) 2011, 2013, 2014, 2016, 2021 Michael L. Gran

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
#define _GNU_SOURCE
#include <config.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <libguile.h>
#include <curl/curl.h>
#include "type.h"

#define SCM_C_LIST_REF(x,i) scm_list_ref(x,scm_from_size_t(i))
#define SCM_C_LIST_LENGTH(x) scm_to_size_t(scm_length(x))
#define SCM_IS_LIST(x) scm_is_true(scm_list_p(x))

static scm_t_bits handle_tag;

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

  assert (x != NULL);

  SCM_NEWSMOB (s_handle, handle_tag, x);

  assert (x == (handle_post_t *) SCM_SMOB_DATA (s_handle));

  if (0)
    {
      fprintf (stderr, "Making <#handle %p>\n", x);
      fprintf (stderr, "\t        handle %p\n", x->handle);
      fprintf (stderr, "\t    postfields %p\n", x->postfields);
      fprintf (stderr, "\t          mime %p\n", x->mime);
      fprintf (stderr, "\t    httpheader %p\n", x->httpheader);
      fprintf (stderr, "\thttp200aliases %p\n", x->http200aliases);
      fprintf (stderr, "\t     mail_rcpt %p\n", x->mail_rcpt);
      fprintf (stderr, "\t         quote %p\n", x->quote);
      fprintf (stderr, "\t     postquote %p\n", x->postquote);
      fprintf (stderr, "\t      prequote %p\n", x->prequote);
      fprintf (stderr, "\t       resolve %p\n", x->resolve);
      fprintf (stderr, "\t telnetoptions %p\n", x->telnetoptions);
      fprintf (stderr, "\t     transfers %d\n", x->transfers);
      fflush (stderr);
    }

  return (s_handle);
}

// Handles are equal if they point to the same C structure
SCM
equalp_handle (SCM x1, SCM x2)
{
  SCM ret;
  CURL *handle1;
  CURL *handle2;

  handle1 = (handle_post_t *) SCM_SMOB_DATA (x1);
  handle2 = (handle_post_t *) SCM_SMOB_DATA (x2);

  if ((handle1 == NULL) || (handle2 == NULL))
    ret = SCM_BOOL_F;
  else if ((handle1 != handle2))
    ret = SCM_BOOL_F;
  else
    ret = SCM_BOOL_T;
  return ret;
}

/* Handles are usually freed by Guile's finalizer thread, which can
   still be running while the process exits.  Freeing a handle then
   races with the libraries' own exit-time cleanup (e.g. OpenSSL's
   OPENSSL_cleanup), which corrupts the heap.  So once the process
   starts exiting, handles are no longer freed: cl_stop_handle_cleanup,
   run from an atexit handler, sets EXITING after waiting for any
   cleanup in progress.  */
static pthread_mutex_t cleanup_lock = PTHREAD_MUTEX_INITIALIZER;
static int exiting = 0;

void
cl_stop_handle_cleanup (void)
{
  pthread_mutex_lock (&cleanup_lock);
  exiting = 1;
  pthread_mutex_unlock (&cleanup_lock);
}

static void gc_free_handle_unlocked (SCM handle);

size_t
gc_free_handle (SCM handle)
{
  pthread_mutex_lock (&cleanup_lock);
  if (!exiting)
    gc_free_handle_unlocked (handle);
  pthread_mutex_unlock (&cleanup_lock);
  return 0;
}

static void
gc_free_handle_unlocked (SCM handle)
{
  handle_post_t *x = _scm_to_handle (handle);

  if (0)
    {
      fprintf (stderr, "Freeing <#handle %p>\n", x);
      fprintf (stderr, "\t        handle %p\n", x->handle);
      fprintf (stderr, "\t          mime %p\n", x->mime);
      fprintf (stderr, "\t    httpheader %p\n", x->httpheader);
      fprintf (stderr, "\thttp200aliases %p\n", x->http200aliases);
      fprintf (stderr, "\t     mail_rcpt %p\n", x->mail_rcpt);
      fprintf (stderr, "\t         quote %p\n", x->quote);
      fprintf (stderr, "\t     postquote %p\n", x->postquote);
      fprintf (stderr, "\t      prequote %p\n", x->prequote);
      fprintf (stderr, "\t       resolve %p\n", x->resolve);
      fprintf (stderr, "\t telnetoptions %p\n", x->telnetoptions);
      fprintf (stderr, "\t     transfers %d\n", x->transfers);
      fflush (stderr);
    }

  if (x != NULL)
    {
      if (x->postfields != NULL)
        {
          free (x->postfields);
          x->postfields = NULL;
          x->postfieldsize = 0;
        }
      if (x->mime != NULL)
        {
          /* curl_mime_free detaches the form from the handle first.  */
          curl_mime_free (x->mime);
          x->mime = NULL;
        }
      if (x->httpheader != NULL)
        {
          curl_slist_free_all (x->httpheader);
          x->httpheader = NULL;
        }
      if (x->http200aliases != NULL)
        {
          curl_slist_free_all (x->http200aliases);
          x->http200aliases = NULL;
        }
      if (x->mail_rcpt != NULL)
        {
          curl_slist_free_all (x->mail_rcpt);
          x->mail_rcpt = NULL;
        }
      if (x->quote != NULL)
        {
          curl_slist_free_all (x->quote);
          x->quote = NULL;
        }
      if (x->postquote != NULL)
        {
          curl_slist_free_all (x->postquote);
          x->postquote = NULL;
        }
      if (x->prequote != NULL)
        {
          curl_slist_free_all (x->prequote);
          x->prequote = NULL;
        }
      if (x->resolve != NULL)
        {
          curl_slist_free_all (x->resolve);
          x->resolve = NULL;
        }
      if (x->telnetoptions != NULL)
        {
          curl_slist_free_all (x->telnetoptions);
          x->telnetoptions = NULL;
        }
      if (x->proxyheader != NULL)
        {
          curl_slist_free_all (x->proxyheader);
          x->proxyheader = NULL;
        }
      if (x->rtspheader != NULL)
        {
          curl_slist_free_all (x->rtspheader);
          x->rtspheader = NULL;
        }
      if (x->connect_to != NULL)
        {
          curl_slist_free_all (x->connect_to);
          x->connect_to = NULL;
        }
      x->transfers = 0;
      x->readport = SCM_BOOL_F;
      x->pending_exception = SCM_BOOL_F;
      if (x->handle != NULL)
        {
          curl_easy_cleanup (x->handle);
          x->handle = NULL;
        }
      /* X itself is GC-allocated; it is reclaimed once unreachable.  */
      x = NULL;
      SCM_SET_SMOB_DATA(handle, NULL);
    }
}

int
#ifdef __GNUC__
print_handle (SCM x, SCM port, scm_print_state *pstate __attribute__ ((unused)))
#else
  print_handle (SCM x, SCM port, scm_print_state *pstate)
#endif
{
  handle_post_t *frm = (handle_post_t *) SCM_SMOB_DATA (x);
  char str[64];

  scm_puts ("#<handle ", port);

  if (frm == (handle_post_t *) NULL)
    {
      scm_puts ("(freed)", port);
    }
  else
    {
      if (snprintf (str, sizeof str, "%p", (void *) frm) < 0)
        scm_puts ("???", port);
      else
        scm_puts (str, port);
    }

  scm_puts (">", port);

  // non-zero means success
  return 1;
}

SCM DLL_PUBLIC
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

/* Unwind handler: free the partially built slist that *DATA points to.  */
static void
free_partial_slist (void *data)
{
  struct curl_slist **slist = data;
  curl_slist_free_all (*slist);
  *slist = NULL;
}

struct curl_slist *
_scm_convert_to_slist (SCM x)
{
  int i, n;
  SCM elt;
  char *str;
  struct curl_slist *slist = NULL;
  struct curl_slist *new_slist;

  if (!_scm_can_convert_to_slist (x))
    return NULL;

  n = scm_to_int (scm_length (x));
  if (n == 0)
    return NULL;

  /* scm_to_locale_string can throw (e.g. a character the locale cannot
     encode), as can the errors below.  Free the partial list if so.  */
  scm_dynwind_begin (0);
  scm_dynwind_unwind_handler (free_partial_slist, &slist, 0);
  for (i = 0; i < n; i ++)
    {
      elt = SCM_C_LIST_REF(x, i);
      if (scm_is_string (elt))
        {
          str = scm_to_locale_string (elt);
          /* On failure this returns NULL but leaves SLIST intact.  */
          new_slist = curl_slist_append (slist, str);
          /* curl_slist_append copies the string.  */
          free (str);
          if (new_slist == NULL)
            {
              /* Out of memory. */
              scm_misc_error ("curl-easy-setopt",
                              "failed to create a Curl list of strings from a Guile list of strings",
                              SCM_EOL);
            }
          slist = new_slist;
        }
      else
        {
          /* This should be impossible. */
          scm_misc_error ("curl-easy-setopt",
                          "failed to create a Curl list of strings from a Guile list that contained non-string data",
                          SCM_EOL);
        }
    }
  scm_dynwind_end ();
  return slist;
}

/* For Guile-1.8, tests to see if this is a string.  For Guile-2.0,
   tests to see if this is a string or bytevector.  */

#define SCM_C_STRING_REF(s,i) \
  scm_to_int (scm_char_to_integer (scm_c_string_ref (s, i)))

int
_scm_can_convert_to_byte_data (SCM x)
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
  return 0;
}

/* For Guile-1.8, returns the memory location of the chars of a
   string.  For Guile-2.0, returns the memory location of the bytes of
   a bytevector. Since these are SCM objects, they are GC'd.  */
uint8_t *
_scm_convert_to_byte_data (SCM x, size_t *len)
{
  if (scm_is_string (x))
    {
      size_t i;
      uint8_t *buf;

      *len = scm_c_string_length (x);
      if (*len == 0)
        return NULL;

      buf = scm_malloc (*len);
      for (i = 0; i < *len; i ++)
        {
          buf[i] = SCM_C_STRING_REF (x, i);
        }
      return buf;
    }
  else
    {
      size_t i;
      uint8_t *buf;

      *len = SCM_BYTEVECTOR_LENGTH (x);
      if (*len == 0)
        return NULL;

      buf = scm_malloc (*len);
      for (i = 0; i < *len; i ++)
        {
          buf[i] = SCM_BYTEVECTOR_CONTENTS (x)[i];
        }
      return buf;
    }
}


/* A form, as given to CURLOPT_MIMEPOST, is a list of fields.  Each
   field is a list of 2 to 4 elements:
   1. name - string
   2. data - string of 8-bit characters, or bytevector
   3. MIME type - string, optional
   4. filename - string, optional; the field is then sent as a file
*/
static int
_scm_can_convert_to_mime_field (SCM x)
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
  if (n >= 3 && !scm_is_string (SCM_C_LIST_REF(x, 2)))
    return 0;
  if (n == 4 && !scm_is_string (SCM_C_LIST_REF(x, 3)))
    return 0;

  return 1;
}

int
_scm_can_convert_to_mime (SCM x)
{
  size_t i, n;

  if (!SCM_IS_LIST(x))
    return 0;

  n = SCM_C_LIST_LENGTH(x);
  for (i = 0; i < n; i ++)
    if (!_scm_can_convert_to_mime_field (SCM_C_LIST_REF(x, i)))
      return 0;
  return 1;
}

/* Unwind handler: free the partially built form that *DATA points to.  */
static void
free_partial_mime (void *data)
{
  curl_mime **mime = data;
  curl_mime_free (*mime);
  *mime = NULL;
}

/* Convert S, the WHAT of the form field at INDEX, to a C string that is
   freed when the current dynwind context ends.  curl takes these as C
   strings, so a NUL character would silently truncate them.  */
static char *
mime_field_string (SCM s, size_t index, const char *what)
{
  char *str;

  if (scm_is_true (scm_string_index (s, SCM_MAKE_CHAR ('\0'),
                                     SCM_UNDEFINED, SCM_UNDEFINED)))
    scm_misc_error ("curl-easy-setopt",
                    "the ~A of the form field at index ~A contains a NUL character",
                    scm_list_2 (scm_from_utf8_string (what),
                                scm_from_size_t (index)));
  str = scm_to_locale_string (s);
  scm_dynwind_free (str);
  return str;
}

static void
check_mime_result (CURLcode rc, size_t index)
{
  if (rc != CURLE_OK)
    scm_misc_error ("curl-easy-setopt",
                    "could not add the form field at index ~A: ~A",
                    scm_list_2 (scm_from_size_t (index),
                                scm_from_locale_string (curl_easy_strerror (rc))));
}

/* Convert X, a list of form fields, to a curl MIME form for the easy
   handle HANDLE.  curl copies every name, data, type and filename into
   the form, so nothing here needs to outlive this call.  Returns NULL
   for an empty list, which clears CURLOPT_MIMEPOST.  The caller owns the
   result and frees it with curl_mime_free.  */
curl_mime *
_scm_convert_to_mime (SCM x, CURL *handle)
{
  curl_mime *mime;
  size_t j, m, n;

  n = SCM_C_LIST_LENGTH (x);
  if (n == 0)
    return NULL;

  mime = curl_mime_init (handle);
  if (mime == NULL)
    scm_misc_error ("curl-easy-setopt", "could not create a form", SCM_EOL);

  /* Free the form if any conversion below throws.  */
  scm_dynwind_begin (0);
  scm_dynwind_unwind_handler (free_partial_mime, &mime, 0);
  for (j = 0; j < n; j ++)
    {
      SCM elt = SCM_C_LIST_REF (x, j);
      curl_mimepart *part;
      uint8_t *contents;
      size_t contents_len;

      m = SCM_C_LIST_LENGTH (elt);

      /* Strings converted for this field are freed when it is done.  */
      scm_dynwind_begin (0);

      part = curl_mime_addpart (mime);
      if (part == NULL)
        check_mime_result (CURLE_OUT_OF_MEMORY, j);

      check_mime_result (curl_mime_name (part,
                                         mime_field_string (SCM_C_LIST_REF (elt, 0),
                                                            j, "name")),
                         j);

      contents = _scm_convert_to_byte_data (SCM_C_LIST_REF (elt, 1),
                                            &contents_len);
      if (contents != NULL)
        scm_dynwind_free (contents);
      /* Empty data converts to NULL; give curl an empty buffer.  */
      check_mime_result (curl_mime_data (part,
                                         contents ? (char *) contents : "",
                                         contents_len),
                         j);

      if (m >= 3)
        check_mime_result (curl_mime_type (part,
                                           mime_field_string (SCM_C_LIST_REF (elt, 2),
                                                              j, "MIME type")),
                           j);
      if (m == 4)
        check_mime_result (curl_mime_filename (part,
                                               mime_field_string (SCM_C_LIST_REF (elt, 3),
                                                                  j, "filename")),
                           j);

      scm_dynwind_end ();
    }
  scm_dynwind_end ();
  return mime;
}

void DLL_PUBLIC
cl_init_type ()
{
  handle_tag = scm_make_smob_type ("handle", sizeof (handle_post_t));
  scm_set_smob_free (handle_tag, gc_free_handle);
  scm_set_smob_print (handle_tag, print_handle);
  scm_set_smob_equalp (handle_tag, equalp_handle);
  scm_c_define_gsubr ("%curl-easy-handle?", 1, 0, 0, cl_is_handle_p);
}
