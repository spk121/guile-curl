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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <libguile.h>
#include <curl/curl.h>
#include "type.h"

#define SCM_C_LIST_REF(x,i) scm_list_ref(x,scm_from_size_t(i))
#define SCM_C_LIST_LENGTH(x) scm_to_size_t(scm_length(x))
#define SCM_IS_LIST(x) scm_is_true(scm_list_p(x))

static scm_t_bits handle_tag;

/* Used to implement the Scheme input-port-based data source for a
   MIME part's body. */
typedef struct mime_port_t
{
  SCM port;
  curl_off_t size;
  curl_off_t position;
} mime_port_t;

int print_handle (SCM x, SCM port, scm_print_state *pstate);

static void
_scm_convert_to_mime_part (curl_mimepart *part, SCM alist, CURL *curl);
static struct curl_mime *
_scm_convert_to_mime_subparts (CURL *curl, SCM subparts_list);
static size_t
read_scm_port (char *buffer, size_t size, size_t nitems, void *arg);
static void
free_scm_port(void *arg);


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

size_t
gc_free_handle (SCM handle)
{
  handle_post_t *x = _scm_to_handle (handle);

  if (0)
    {
      fprintf (stderr, "Freeing <#handle %p>\n", x);
      fprintf (stderr, "\t        handle %p\n", x->handle);
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
      if (x->mimepost != NULL)
        {
          curl_mime_free (x->mimepost);
          x->mimepost = NULL;
        }
      x->transfers = 0;
      if (x->handle != NULL)
        {
          curl_easy_cleanup (x->handle);
          x->handle = NULL;
        }
      free (x);
      x = NULL;
      SCM_SET_SMOB_DATA(handle, NULL);
    }
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
    return 1;                   /* Empty slist */
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

  if (!_scm_can_convert_to_slist (x))
    return NULL;

  n = scm_to_int (scm_length (x));
  if (n == 0)
    return NULL;
  for (i = 0; i < n; i ++)
    {
      elt = SCM_C_LIST_REF(x, i);
      if (scm_is_string (elt))
        {
          str = scm_to_locale_string (elt);
          slist = curl_slist_append (slist, str);
          if (slist == NULL)
            {
              /* This should be impossible. */
              scm_error (SCM_BOOL_F,
                         "_scm_convert_to_slist",
                         "failed to create a Curl list of strings from a Guile list of strings",
                         SCM_BOOL_F,
                         SCM_BOOL_F);
              break;
            }
        }
      else
        {
          /* This should be impossible. */
          scm_error (SCM_BOOL_F,
                     "_scm_convert_to_slist",
                     "failed to create a Curl list of strings from a Guile list that contained non-string data",
                     SCM_BOOL_F,
                     SCM_BOOL_F);
          break;
        }
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
  int i, n;
  int c;

  if (scm_is_bytevector (x))
    return 1;
  else if (scm_is_string (x))
    {
      n = scm_c_string_length (x);
      if (n == 0)
        return 1;               /* Empty string */
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

/* Each entry in a MIME post is an alist that can have these entries
   (name . [string])
   (type . [string])
   (data . [string or bytevector])
   (subparts . [mime])
   (filename . [string])     This is the intended remote filename
   (filedata . [string])     This is the name of a local file to read data from
   (encoder . [string])      String is one of 'binary' '8bit' '7bit' 'base64' 'quoted-printable'
   (headers . [list of strings])
   (port [datasize] [port])
*/
static int
_scm_can_convert_to_mimepost_entry (SCM x)
{
  size_t i, n;

  if (!SCM_IS_LIST(x))
    return 0;

  n = SCM_C_LIST_LENGTH (x);
  if (n == 0)
    return 0;
  for (i = 0; i < n; i ++)
    {
      SCM entry = SCM_C_LIST_REF (x, i);
      SCM key;
      if (scm_is_false (scm_pair_p (entry)))
        return 0;
      key = scm_car (entry);
      if (scm_is_false (scm_symbol_p (key)))
        return 0;

      if (scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("name")))
          || scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("type")))
          || scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("filename")))
          || scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("filedata")))
          || scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("encoder"))))
        {
          if (scm_is_false (scm_string_p (scm_cdr (entry))))
            return 0;
        }
      else if (scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("data"))))
        {
          SCM data = scm_cdr (entry);
          if (scm_is_false (scm_string_p (data)) && scm_is_false (scm_bytevector_p (data)))
            return 0;
        }
      else if (scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("headers"))))
        {
          SCM headers = scm_cdr (entry);
          size_t j, hn;
          if (!SCM_IS_LIST (headers))
            return 0;
          hn = SCM_C_LIST_LENGTH (headers);
          for (j = 0; j < hn; j ++)
            {
              if (scm_is_false (scm_string_p (SCM_C_LIST_REF (headers, j))))
                return 0;
            }
        }
      else if (scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("port"))))
        {
          int64_t datasize_i64;
          curl_off_t datasize;

          if (scm_to_int (scm_length (entry)) != 3)
            return 0;
          if (scm_is_false (scm_integer_p (scm_cadr (entry))))
            return 0;

          datasize_i64 = scm_to_int64 (scm_cadr (entry));
          if (datasize_i64 < 0)
            return 0;
          datasize = (curl_off_t) datasize_i64;
          if ((int64_t) datasize != datasize_i64)
            return 0;

          if (scm_is_false (scm_input_port_p (scm_caddr (entry))))
            return 0;
        }
      else if (scm_is_true (scm_eq_p (key, scm_from_utf8_symbol ("subparts"))))
        {
          SCM subparts = scm_cdr (entry);
          int spi, spn;
          /* subparts should be a list of MIME part entries */
          if (!SCM_IS_LIST (subparts))
            return 0;
          spn = SCM_C_LIST_LENGTH (subparts);
          if (spn == 0)
            return 0;  /* subparts list must be non-empty */
          for (spi = 0; spi < spn; spi++)
            {
              if (!_scm_can_convert_to_mimepost_entry (SCM_C_LIST_REF (subparts, spi)))
                return 0;
            }
        }
      else
        return 0;
    }
  return 1;
}

int
_scm_can_convert_to_mimepost (SCM x)
{
  int i, n;
  SCM elt;

  if (!SCM_IS_LIST(x))
    return 0;

  n = SCM_C_LIST_LENGTH(x);
  if (n == 0)
    return 0;                   /* Empty post is invalid */
  for (i = 0; i < n; i ++)
    {
      elt = SCM_C_LIST_REF(x, i);
      if (!_scm_can_convert_to_mimepost_entry (elt))
          return 0;
    }
  return 1;
}

struct curl_mime *
_scm_convert_to_mime (CURL *curl, SCM x)
{
  curl_mime *mime;
  int j,m;

  assert (curl != NULL);

  // Create the mime handle
  mime = curl_mime_init (curl);
  if (mime == NULL)
    scm_misc_error ("%list->mime", "MIME handle allocation failure", SCM_EOL);

  // X should be a list of alists
  if (scm_is_false (scm_list_p (x)))
    scm_wrong_type_arg_msg ("%list->mime", 0, x, "list");
  m = scm_to_int (scm_length (x));
  if (m == 0)
    scm_wrong_type_arg_msg ("%list->mime", 0, x, "non-empty list");
  if (m > 1024)
    scm_wrong_type_arg_msg ("%list->mime", 0, x, "list of length 1024 or less");
  for (j = 0; j < m; j ++)
    {
      curl_mimepart *part;
      SCM spart;
      int k, n;

      // Each element should be an alist
      spart = scm_list_ref (x, scm_from_int (j));
      if (scm_is_false (scm_list_p (spart)))
        scm_wrong_type_arg_msg ("%list->mime", 0, spart, "list");
      n = scm_to_int (scm_length (spart));
      if (n == 0)
        scm_wrong_type_arg_msg ("%list->mime", 0, spart, "non-empty list");
      part = curl_mime_addpart(mime);
      if (part == NULL)
        scm_misc_error ("%list->mime", "MIME part allocation failure", SCM_EOL);

      _scm_convert_to_mime_part (part, spart, curl);
    }
  return mime;
}

static struct curl_mime *
_scm_convert_to_mime_subparts (CURL *curl, SCM subparts_list)
{
  curl_mime *mime;
  int j, m;

  assert (curl != NULL);

  /* Create a mime handle for subparts */
  mime = curl_mime_init (curl);
  if (mime == NULL)
    scm_misc_error ("%list->mime-subparts", "MIME handle allocation failure", SCM_EOL);

  /* subparts_list should be a list of alists */
  if (scm_is_false (scm_list_p (subparts_list)))
    scm_wrong_type_arg_msg ("%list->mime-subparts", 0, subparts_list, "list");
  m = scm_to_int (scm_length (subparts_list));
  if (m == 0)
    scm_wrong_type_arg_msg ("%list->mime-subparts", 0, subparts_list, "non-empty list");
  if (m > 1024)
    scm_wrong_type_arg_msg ("%list->mime-subparts", 0, subparts_list, "list of length 1024 or less");

  for (j = 0; j < m; j++)
    {
      curl_mimepart *subpart;
      SCM spart;
      int k, n;

      /* Each element should be an alist */
      spart = scm_list_ref (subparts_list, scm_from_int (j));
      if (scm_is_false (scm_list_p (spart)))
        {
          curl_mime_free (mime);
          scm_wrong_type_arg_msg ("%list->mime-subparts", 0, spart, "list");
        }
      n = scm_to_int (scm_length (spart));
      if (n == 0)
        {
          curl_mime_free (mime);
          scm_wrong_type_arg_msg ("%list->mime-subparts", 0, spart, "non-empty list");
        }
      subpart = curl_mime_addpart (mime);
      if (subpart == NULL)
        {
          curl_mime_free (mime);
          scm_misc_error ("%list->mime-subparts", "MIME part allocation failure", SCM_EOL);
        }

      _scm_convert_to_mime_part (subpart, spart, curl);
    }
  return mime;
}

static void
_scm_convert_to_mime_part (curl_mimepart *part, SCM alist, CURL *curl)
{
  int k, n;
  int name_found = 0;
  int type_found = 0;
  int data_found = 0;
  int filedata_found = 0;
  int port_found = 0;
  int subparts_found = 0;
  int filename_found = 0;
  int encoder_found = 0;
  int headers_found = 0;

  // As a precondition, we know alist is a non-empty list.
  n = scm_to_int (scm_length (alist));
  for (k = 0; k < n; k ++)
    {
      char *key;
      size_t key_len;
      SCM entry;
      SCM skey;

      entry = scm_list_ref (alist, scm_from_int (k));
      if (scm_is_false (scm_pair_p (entry)))
        scm_wrong_type_arg_msg ("%list->mime-part", 0, entry, "pair");
      if (scm_is_false (scm_symbol_p (scm_car (entry))))
        scm_wrong_type_arg_msg ("%list->mime-part", 0, scm_car (entry), "symbol");

      key = scm_to_utf8_stringn (scm_symbol_to_string (scm_car (entry)), &key_len);

      if (strcmp (key, "name") == 0)
        {
          char *name;
          size_t name_len;
          CURLcode code;

          if (name_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: name", SCM_EOL);
            }
          if (scm_is_false (scm_string_p (scm_cdr (entry))))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, scm_cdr (entry), "string");
            }
          name = scm_to_utf8_stringn (scm_cdr (entry), &name_len);
          code = curl_mime_name (part, name);
          free (name);
          if (code != CURLE_OK)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "failed to set MIME part name: ~A", scm_list_1 (scm_cdr (entry)));
            }
          name_found = 1;
        }
      else if (strcmp (key, "type") == 0)
        {
          char *type;
          size_t type_len;
          CURLcode code;

          if (type_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: type", SCM_EOL);
            }
          if (scm_is_false (scm_string_p (scm_cdr (entry))))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, scm_cdr (entry), "string");
            }
          type = scm_to_utf8_stringn (scm_cdr (entry), &type_len);
          code = curl_mime_type (part, type);
          free (type);
          if (code != CURLE_OK)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "failed to set MIME part type: ~A", scm_list_1 (scm_cdr (entry)));
            }
          type_found = 1;
        }
      else if (strcmp (key, "data") == 0)
        {
          SCM sdata;
          unsigned char *data;
          size_t data_len;
          CURLcode code;

          if (data_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: data", SCM_EOL);
            }
          if (filedata_found || port_found || subparts_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "MIME part content must specify exactly one of data, filedata, port, or subparts", SCM_EOL);
            }
          sdata = scm_cdr (entry);
          if (scm_is_false (scm_string_p (sdata)) && scm_is_false (scm_bytevector_p (sdata)))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, sdata, "string or bytevector");
            }
          if (scm_is_true (scm_string_p (sdata)))
            {
              data = scm_to_utf8_stringn (sdata, &data_len);
              code = curl_mime_data (part, data, data_len);
              free (data);
            }
          else
            code = curl_mime_data (part, SCM_BYTEVECTOR_CONTENTS (sdata), SCM_BYTEVECTOR_LENGTH (sdata));
          if (code != CURLE_OK)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "failed to set MIME part data: ~A", scm_list_1 (scm_cdr (entry)));
            }
          data_found = 1;
        }
      else if (strcmp (key, "filename") == 0)
        {
          char *filename;
          size_t filename_len;
          CURLcode code;

          if (filename_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: filename", SCM_EOL);
            }
          if (scm_is_false (scm_string_p (scm_cdr (entry))))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, scm_cdr (entry), "string");
            }
          filename = scm_to_utf8_stringn (scm_cdr (entry), &filename_len);
          code = curl_mime_filename (part, filename);
          free (filename);
          if (code != CURLE_OK)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "failed to set MIME part filename: ~A", scm_list_1 ( scm_cdr (entry)));
            }
          filename_found = 1;
        }
      else if (strcmp (key, "filedata") == 0)
        {
          char *filename;
          size_t filename_len;
          CURLcode code;

          if (filedata_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: filedata", SCM_EOL);
            }
          if (data_found || port_found || subparts_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "MIME part content must specify exactly one of data, filedata, port, or subparts", SCM_EOL);
            }
          if (scm_is_false (scm_string_p (scm_cdr (entry))))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, scm_cdr (entry), "string");
            }
          filename = scm_to_utf8_stringn (scm_cdr (entry), &filename_len);
          code = curl_mime_filedata (part, filename);
          free (filename);
          if (code != CURLE_OK)
            {
              SCM cerr = scm_from_utf8_string (curl_easy_strerror (code));
              scm_misc_error ("%list->mime-part", "failed to set MIME part filedata: ~A: ~A",
                              scm_list_2 (cerr, scm_cdr (entry)));
            }
          filedata_found = 1;
        }
      else if (strcmp (key, "encoder") == 0)
        {
          char *encoder;
          size_t encoder_len;
          CURLcode code;

          if (encoder_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: encoder", SCM_EOL);
            }
          if (scm_is_false (scm_string_p (scm_cdr (entry))))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, scm_cdr (entry), "string");
            }
          encoder = scm_to_utf8_stringn (scm_cdr (entry), &encoder_len);
          code = curl_mime_encoder (part, encoder);
          free (encoder);
          if (code != CURLE_OK)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "failed to set MIME part encoder: ~A", scm_list_1 (scm_cdr (entry)));
            }
          encoder_found = 1;
        }
      else if (strcmp (key, "headers") == 0)
        {
          SCM sheaders, shdr;
          int h;
          int headers_len;
          char *hdr;
          size_t hdr_len;
          struct curl_slist *headers = NULL;
          CURLcode code;

          if (headers_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: headers", SCM_EOL);
            }
          sheaders = scm_cdr (entry);
          if (scm_is_false (scm_list_p (sheaders)))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, sheaders, "list");
            }
          headers_len = scm_to_int (scm_length (sheaders));
          for (h = 0; h < headers_len; h ++)
            {
              shdr = scm_list_ref (sheaders, scm_from_int (h));
              if (scm_is_false (scm_string_p (shdr)))
                {
                  free (key);
                  scm_wrong_type_arg_msg ("%list->mime-part", 0, shdr, "string");
                }
            }

          // Convert a scheme list to a curl slist
          for (h = 0; h < headers_len; h ++)
            {
              struct curl_slist *temp = NULL;
              shdr = scm_list_ref (sheaders, scm_from_int (h));
              hdr = scm_to_utf8_stringn (shdr, &hdr_len);
              temp = curl_slist_append (headers, hdr);
              free (hdr);
              if (!temp)
                {
                  curl_slist_free_all (headers);
                  scm_misc_error ("%list->mime-part", "Out of memory", SCM_EOL);
                }
              headers = temp;
            }
          /* Let libcurl own the header list for this MIME part. */
          code = curl_mime_headers (part, headers, 1);
          if (code != CURLE_OK)
            {
              free (key);
              curl_slist_free_all (headers);
              scm_misc_error ("%list->mime-part", "failed to set MIME part headers: ~A", scm_list_1 (sheaders));
            }
          headers_found = 1;
        }
      else if (strcmp (key, "port") == 0)
        {
          // We use the curl_mime_data_cb to handle ports.
          SCM sport, sdatasize;
          int64_t datasize_i64;
          curl_off_t datasize;
          CURLcode code;
          mime_port_t *mime_port;

          if (port_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: port", SCM_EOL);
            }
          if (data_found || filedata_found || subparts_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "MIME part content must specify exactly one of data, filedata, port, or subparts", SCM_EOL);
            }
          if (scm_to_int (scm_length (entry)) != 3)
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, entry, "list of 3 elements");
            }
          if (scm_is_false (scm_integer_p (scm_cadr (entry))))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, scm_cadr (entry), "integer");
            }
          if (scm_is_false (scm_input_port_p (scm_caddr (entry))))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, scm_caddr (entry), "input port");
            }
          sdatasize = scm_cadr (entry);
          sport = scm_caddr (entry);
            datasize_i64 = scm_to_int64 (sdatasize);
            if (datasize_i64 < 0)
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, sdatasize, "non-negative integer");
            }
            datasize = (curl_off_t) datasize_i64;
            if ((int64_t) datasize != datasize_i64)
              {
                free (key);
                scm_wrong_type_arg_msg ("%list->mime-part", 0, sdatasize,
                                        "integer representable as curl_off_t");
              }

          mime_port = malloc (sizeof (mime_port_t));
          if (mime_port == NULL)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "out of memory", SCM_EOL);
            }
          mime_port->port = sport;
          mime_port->size = datasize;
          mime_port->position = 0;
          code = curl_mime_data_cb (part, datasize, read_scm_port, NULL, free_scm_port, mime_port);
          if (code != CURLE_OK)
            {
              free (key);
              free (mime_port);
              scm_misc_error ("%list->mime-part", "failed to set MIME part callbacks for Scheme ports", SCM_EOL);
            }
          scm_gc_protect_object (mime_port->port);
          port_found = 1;
        }
      else if (strcmp (key, "subparts") == 0)
        {
          SCM ssubparts;
          CURLcode code;
          curl_mime *subparts_mime;

          if (subparts_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "duplicate MIME part field: subparts", SCM_EOL);
            }
          if (data_found || filedata_found || port_found)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "MIME part content must specify exactly one of data, filedata, port, or subparts", SCM_EOL);
            }
          ssubparts = scm_cdr (entry);
          if (scm_is_false (scm_list_p (ssubparts)))
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, ssubparts, "list");
            }
          if (scm_to_int (scm_length (ssubparts)) == 0)
            {
              free (key);
              scm_wrong_type_arg_msg ("%list->mime-part", 0, ssubparts, "non-empty list");
            }
          /* Use the curl handle passed as parameter to create subparts mime structure */
          if (curl == NULL)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "internal error: curl handle not available for subparts", SCM_EOL);
            }
          subparts_mime = _scm_convert_to_mime_subparts (curl, ssubparts);
          if (subparts_mime == NULL)
            {
              free (key);
              scm_misc_error ("%list->mime-part", "failed to create MIME subparts structure", SCM_EOL);
            }
          code = curl_mime_subparts (part, subparts_mime);
          if (code != CURLE_OK)
            {
              curl_mime_free (subparts_mime);
              free (key);
              scm_misc_error ("%list->mime-part", "failed to set MIME part subparts: ~A",
                              scm_list_1 (scm_from_utf8_string (curl_easy_strerror (code))));
            }
          subparts_found = 1;
        }
      else
        {
          free (key);
          scm_misc_error ("%list->mime-part", "unknown MIME part field: ~S",
                          scm_list_1 (scm_car (entry)));
        }
      free (key);
    }
  if (!name_found)
    scm_misc_error ("%list->mime-part",
                    "missing required MIME part field: name", SCM_EOL);
  if (!(data_found || filedata_found || port_found || subparts_found))
    scm_misc_error ("%list->mime-part",
                    "missing required MIME part content (data, filedata, port, or subparts)",
                    SCM_EOL);

  return;
}

static size_t
read_scm_port (char *buffer, size_t size, size_t nitems, void *arg)
{
  mime_port_t *p;
  curl_off_t remaining;
  size_t to_read;
  size_t n;

  if (buffer == NULL || arg == NULL)
    scm_misc_error ("%read-scm-port", "internal error", SCM_EOL);

  p = (mime_port_t *) arg;
  to_read = size * nitems;
  if (p->position >= p->size)
    return 0;
  remaining = p->size - p->position;
  if ((curl_off_t) to_read > remaining)
    to_read = (size_t) remaining;
  n = scm_c_read (p->port, buffer, to_read);
  if (n == 0)
    return 0;
  p->position += (curl_off_t) n;
  return n;
}

static void
free_scm_port (void *arg)
{
  mime_port_t *p;

  if (arg == NULL)
    return;

  p = (mime_port_t *) arg;
  scm_gc_unprotect_object (p->port);
  free (p);
  p = NULL;
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
