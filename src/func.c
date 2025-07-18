/* Functions for guile-curl

   Copyright (c) 2011, 2013, 2014, 2016, 2025 Michael L. Gran

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
#include <config.h>
#include <libguile.h>
#include <curl/curl.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "type.h"
#include "func.h"

static size_t write_callback (void *ptr, size_t size, size_t nmemb,
                              void *port);
static size_t read_callback (void *ptr, size_t size, size_t nmemb,
                             void *port);

CURLcode error_code = CURLE_OK;
char error_string[CURL_ERROR_SIZE+1] = "";

struct scm_flag
{
  SCM scm;
  int flag;
};

SCM DLL_PUBLIC cl_easy_init ()
{
  CURL *handle;
  handle_post_t *hp;

  handle = curl_easy_init ();

  if (handle == NULL)
    {
      scm_error (SCM_BOOL_F,
                 "curl-easy-init",
                 "initialization failure",
                 SCM_BOOL_F,
                 SCM_BOOL_F);
    }

  hp = scm_malloc (sizeof (handle_post_t));
  memset (hp, 0, sizeof (handle_post_t));
  hp->handle = handle;

  if (0)
    {
      fprintf (stderr, "Allocating <#handle %p>\n", hp);
      fprintf (stderr, "\t        handle %p\n", hp->handle);
      fprintf (stderr, "\t      httppost %p\n", hp->httppost);
      fprintf (stderr, "\t    httpheader %p\n", hp->httpheader);
      fprintf (stderr, "\thttp200aliases %p\n", hp->http200aliases);
      fprintf (stderr, "\t     mail_rcpt %p\n", hp->mail_rcpt);
      fprintf (stderr, "\t         quote %p\n", hp->quote);
      fprintf (stderr, "\t     postquote %p\n", hp->postquote);
      fprintf (stderr, "\t      prequote %p\n", hp->prequote);
      fprintf (stderr, "\t       resolve %p\n", hp->resolve);
      fprintf (stderr, "\t telnetoptions %p\n", hp->telnetoptions);
      fprintf (stderr, "\t     transfers %d\n", hp->transfers);
      fflush (stderr);
    }

  return _scm_from_handle (hp);
}

SCM DLL_PUBLIC
cl_easy_setopt (SCM handle, SCM option, SCM param, SCM big)
{
  handle_post_t *c_handle;
  CURLoption c_option;
  CURLcode code = CURLE_UNSUPPORTED_PROTOCOL;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "curl-easy-setopt");
  SCM_ASSERT (scm_is_integer (option), option, SCM_ARG2, "curl-easy-setopt");

  c_handle = _scm_to_handle (handle);
  c_option = (CURLoption) scm_to_int (option);

  if (c_option == CURLOPT_POSTFIELDS)
    {
      if (_scm_can_convert_to_byte_data (param))
        {
          size_t len;
          uint8_t *m = _scm_convert_to_byte_data (param, &len);
          free (c_handle->postfields);
          c_handle->postfields = m;
          curl_easy_setopt (c_handle->handle, CURLOPT_POSTFIELDSIZE, len);
          c_handle->postfieldsize = len;
          code = curl_easy_setopt (c_handle->handle, CURLOPT_POSTFIELDS, (char *) m);
        }
      else
        scm_error (SCM_BOOL_F, "cl-easy-setopt", "CURLOPT_POSTFIELDS requires 8-bit string or bytevector data",
                   SCM_BOOL_F, SCM_BOOL_F);
    }
  else if (c_option == CURLOPT_HTTPHEADER)
    {
      if (_scm_can_convert_to_slist (param))
        {
          /* slists require special handling to free them properly, so
             they are stored with the Curl handle.  */
          struct curl_slist *sl = _scm_convert_to_slist (param);
          if (c_handle->httpheader)
            curl_slist_free_all (c_handle->httpheader);
          c_handle->httpheader = sl;
          code = curl_easy_setopt (c_handle->handle, CURLOPT_HTTPHEADER, sl);
        }
      else
        scm_error (SCM_BOOL_F, "cl-easy-setopt", "CURLOPT_HTTPHEADER requires a list of strings",
                   SCM_BOOL_F, SCM_BOOL_F);
    }
  else if (scm_is_integer (param))
    {
      if (scm_is_true (big))
        code = curl_easy_setopt (c_handle->handle, c_option, scm_to_int64 (param));
      else
        code = curl_easy_setopt (c_handle->handle, c_option, scm_to_long (param));
    }
  else if (scm_is_string (param))
    {
      /* Strings are copied by curl, so they can be freed here. */
      char *str;
      str = scm_to_locale_string (param);
      code = curl_easy_setopt (c_handle->handle, c_option, str);
      free (str);
    }
  else if (_scm_can_convert_to_slist (param))
    {
      /* slists require special handling to free them properly, so
         they are stored with the Curl handle.  */
      struct curl_slist *sl = _scm_convert_to_slist (param);
      int ok = 1;
      if (c_option == CURLOPT_HTTP200ALIASES)
        {
          if (c_handle->http200aliases)
            curl_slist_free_all (c_handle->http200aliases);
          c_handle->http200aliases = sl;
        }
      else if (c_option == CURLOPT_MAIL_RCPT)
        {
          if (c_handle->mail_rcpt)
            curl_slist_free_all (c_handle->mail_rcpt);
          c_handle->mail_rcpt = sl;
        }
      else if (c_option == CURLOPT_QUOTE)
        {
          if (c_handle->quote)
            curl_slist_free_all (c_handle->quote);
          c_handle->quote = sl;
        }
      else if (c_option == CURLOPT_POSTQUOTE)
        {
          if (c_handle->postquote)
            curl_slist_free_all (c_handle->postquote);
          c_handle->postquote = sl;
        }
      else if (c_option == CURLOPT_PREQUOTE)
        {
          if (c_handle->prequote)
            curl_slist_free_all (c_handle->prequote);
          c_handle->prequote = sl;
        }
      else if (c_option == CURLOPT_RESOLVE)
        {
          if (c_handle->resolve)
            curl_slist_free_all (c_handle->resolve);
          c_handle->resolve = sl;
        }
      else if (c_option == CURLOPT_TELNETOPTIONS)
        {
          if (c_handle->telnetoptions)
            curl_slist_free_all (c_handle->telnetoptions);
          c_handle->telnetoptions = sl;
        }
      else
        {
          // Bad slist option
          ok = 0;
        }
      if (ok)
        code = curl_easy_setopt (c_handle->handle, c_option, sl);

    }
  else if (_scm_can_convert_to_httppost (param))
    {
      if (c_option == CURLOPT_HTTPPOST)
        {
          struct curl_httppost *p;
          p = _scm_convert_to_httppost (param);
          free (c_handle->httppost);
          c_handle->httppost = p;
          code = curl_easy_setopt (c_handle, CURLOPT_HTTPPOST, p);
        }
    }
  else if (scm_is_true (scm_input_port_p (param)))
    {
      if (c_option == CURLOPT_READDATA)
        {
          curl_easy_setopt (c_handle->handle, CURLOPT_READFUNCTION, read_callback);
          code = curl_easy_setopt (c_handle->handle, CURLOPT_READDATA, SCM2PTR (param));
        }
    }
  else
    scm_error (SCM_BOOL_F,
               "curl-easy-setopt",
               "unimplemented option type",
               SCM_BOOL_F,
               SCM_BOOL_F);
  if (code != CURLE_OK)
    scm_error (SCM_BOOL_F,
               "curl-easy-setopt",
               "bad handle",
               SCM_BOOL_F,
               SCM_BOOL_F);

  return SCM_UNSPECIFIED;
}

SCM DLL_PUBLIC
cl_easy_getinfo (SCM handle, SCM option)
{
  handle_post_t *c_handle;
  CURLoption c_option;
  CURLcode code = CURLE_UNSUPPORTED_PROTOCOL;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "curl-easy-getinfo");
  SCM_ASSERT (scm_is_integer (option), option, SCM_ARG2, "curl-easy-getinfo");

  c_handle = _scm_to_handle (handle);
  c_option = (CURLoption) scm_to_int (option);

  if (c_handle->transfers < 1)
    scm_misc_error ("curl-easy-getinfo",
                    "cannot retrieve information before performing an operation",
                    SCM_EOL);

  if (c_option == CURLINFO_ACTIVESOCKET)
    {
      curl_socket_t c_sockfd;
      code = curl_easy_getinfo(c_handle->handle, c_option, &c_sockfd);
      if (code == CURLE_OK && c_sockfd != CURL_SOCKET_BAD)
#ifdef _WIN32
        return scm_from_unsigned_int (c_sockfd);
#else
      return scm_from_int (c_sockfd);
#endif
      else
        {
          error_code = code;
          return SCM_BOOL_F;
        }
    }
  else if ((c_option == CURLINFO_APPCONNECT_TIME_T)
           || (c_option == CURLINFO_CONNECT_TIME_T)
#ifdef CURLINFO_CONN_ID
           || (c_option == CURLINFO_CONN_ID)
#endif
           || (c_option == CURLINFO_CONTENT_LENGTH_DOWNLOAD_T)
           || (c_option == CURLINFO_CONTENT_LENGTH_UPLOAD_T)
           || (c_option == CURLINFO_FILETIME_T)
           || (c_option == CURLINFO_NAMELOOKUP_TIME_T)
#ifdef CURLINFO_POSTTRANSFER_TIME_T
           || (c_option == CURLINFO_POSTTRANSFER_TIME_T)
#endif
           || (c_option == CURLINFO_PRETRANSFER_TIME_T)
#ifdef CURLINFO_QUEUE_TIME_T
           || (c_option == CURLINFO_QUEUE_TIME_T)
#endif
           || (c_option == CURLINFO_REDIRECT_TIME_T)
           || (c_option == CURLINFO_RETRY_AFTER)
           || (c_option == CURLINFO_SIZE_DOWNLOAD_T)
           || (c_option == CURLINFO_SIZE_UPLOAD_T)
           || (c_option == CURLINFO_SPEED_DOWNLOAD_T)
           || (c_option == CURLINFO_STARTTRANSFER_TIME_T)
           || (c_option == CURLINFO_TOTAL_TIME_T)
           )
    {
      curl_off_t c_val;
      code = curl_easy_getinfo (c_handle->handle, c_option, &c_val);
      if (code == CURLE_OK)
        {
          if (sizeof (curl_off_t) == 4)
            return scm_from_int32 (c_val);
          else if (sizeof (curl_off_t) == 8)
            return scm_from_int64 (c_val);
          else
            {
              error_code = code;
              return SCM_BOOL_F;
            }
        }
    }
  else if ((c_option == CURLINFO_CONDITION_UNMET)
           || (c_option == CURLINFO_FILETIME)
           || (c_option == CURLINFO_HEADER_SIZE)
           || (c_option == CURLINFO_HTTPAUTH_AVAIL)
#ifdef CURLINFO_HTTPAUTH_USED
           || (c_option == CURLINFO_HTTPAUTH_USED)
#endif
           || (c_option == CURLINFO_HTTP_CONNECTCODE)
           || (c_option == CURLINFO_HTTP_VERSION)
           || (c_option == CURLINFO_LOCAL_PORT)
           || (c_option == CURLINFO_NUM_CONNECTS)
           || (c_option == CURLINFO_OS_ERRNO)
           || (c_option == CURLINFO_PRIMARY_PORT)
           || (c_option == CURLINFO_PROXYAUTH_AVAIL)
#ifdef CURLINFO_PROXYAUTH_USED
           || (c_option == CURLINFO_PROXYAUTH_USED)
#endif
           || (c_option == CURLINFO_PROXY_ERROR)
           || (c_option == CURLINFO_PROXY_SSL_VERIFYRESULT)
           || (c_option == CURLINFO_REDIRECT_COUNT)
           || (c_option == CURLINFO_REQUEST_SIZE)
           || (c_option == CURLINFO_RESPONSE_CODE)
           || (c_option == CURLINFO_RTSP_CLIENT_CSEQ)
           || (c_option == CURLINFO_RTSP_CSEQ_RECV)
           || (c_option == CURLINFO_RTSP_SERVER_CSEQ)
           || (c_option == CURLINFO_SSL_VERIFYRESULT)
#ifdef CURLINFO_USED_PROXY
           || (c_option == CURLINFO_USED_PROXY)
#endif
#ifdef CURLINFO_XFER_ID
           || (c_option == CURLINFO_XFER_ID)
#endif
           )
    {
      long c_val;
      code = curl_easy_getinfo (c_handle->handle, c_option, &c_val);
      if (code == CURLE_OK)
        return scm_from_long(c_val);
      else
        {
          error_code = code;
          return SCM_BOOL_F;
        }
    }
  else if ((c_option == CURLINFO_COOKIELIST)
           || (c_option == CURLINFO_SSL_ENGINES))
    {
      struct curl_slist *c_cookies;
      code = curl_easy_getinfo (c_handle->handle, c_option, &c_cookies);
      if (code == CURLE_OK)
        {
          struct curl_slist *c_each;
          SCM lst = SCM_EOL;
          c_each = c_cookies;

          while (c_each)
            {
              lst = scm_cons (scm_from_utf8_string (c_each->data), lst);
              c_each = c_each->next;
            }
          curl_slist_free_all (c_cookies);
          return scm_reverse(lst);
        }
      else
        {
          error_code = code;
          return SCM_BOOL_F;
        }
    }
  else if (0
#ifdef CURLINFO_CAINFO
	   || (c_option == CURLINFO_CAINFO)
#endif
#ifdef CURLINFO_CAPATH
           || (c_option == CURLINFO_CAPATH)
#endif
           || (c_option == CURLINFO_EFFECTIVE_METHOD)
           || (c_option == CURLINFO_EFFECTIVE_URL)
           || (c_option == CURLINFO_FTP_ENTRY_PATH)
           || (c_option == CURLINFO_LOCAL_IP)
           || (c_option == CURLINFO_PRIMARY_IP)
           || (c_option == CURLINFO_REDIRECT_URL)
           || (c_option == CURLINFO_REFERER)
           || (c_option == CURLINFO_RTSP_SESSION_ID)
           || (c_option == CURLINFO_SCHEME)
           )
    {
      char *c_str = NULL;
      code = curl_easy_getinfo (c_handle->handle, c_option, &c_str);
      if (code == CURLE_OK)
        {
          if (c_str == NULL)
            return scm_from_utf8_string ("");
          return scm_from_utf8_string (c_str);
        }
      else
        {
          error_code = code;
          return SCM_BOOL_F;
        }
    }
  else if ((c_option == CURLINFO_APPCONNECT_TIME)
           || (c_option == CURLINFO_CONNECT_TIME)
           || (c_option == CURLINFO_NAMELOOKUP_TIME)
           || (c_option == CURLINFO_PRETRANSFER_TIME)
           || (c_option == CURLINFO_STARTTRANSFER_TIME)
           || (c_option == CURLINFO_REDIRECT_TIME)
           || (c_option == CURLINFO_TOTAL_TIME)
           )
    {
      double c_val;
      code = curl_easy_getinfo (c_handle->handle, c_option, &c_val);
      if (code == CURLE_OK)
        return scm_from_double (c_val);
      else
        {
          error_code = code;
          return SCM_BOOL_F;
        }
    }
  else
    scm_error (SCM_BOOL_F,
               "curl-easy-getinfo",
               "unimplemented option type",
               SCM_BOOL_F,
               SCM_BOOL_F);
  return SCM_BOOL_F;
}


SCM DLL_PUBLIC
cl_easy_perform (SCM handle, SCM bvflag, SCM headerflag)
{
  handle_post_t *c_handle;
  SCM data;
  CURLcode status;
  struct scm_flag body_sf, header_sf;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-easy-perform");

  c_handle = _scm_to_handle (handle);

  body_sf.flag = scm_is_true (bvflag);
  if (body_sf.flag)
    data = scm_c_make_bytevector (0);
  else
    data = scm_c_make_string (0, SCM_MAKE_CHAR('\n'));
  body_sf.scm = data;

  header_sf.flag = 0;
  if (header_sf.flag)
    data = scm_c_make_bytevector (0);
  else
    data = scm_c_make_string (0, SCM_MAKE_CHAR('\n'));
  header_sf.scm = data;

  if (scm_is_true (headerflag))
  {
    curl_easy_setopt (c_handle->handle, CURLOPT_HEADERFUNCTION, write_callback);
    curl_easy_setopt (c_handle->handle, CURLOPT_HEADERDATA, &header_sf);
    curl_easy_setopt (c_handle->handle, CURLOPT_ERRORBUFFER, error_string);
  }

  curl_easy_setopt (c_handle->handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt (c_handle->handle, CURLOPT_WRITEDATA, &body_sf);
  curl_easy_setopt (c_handle->handle, CURLOPT_ERRORBUFFER, error_string);

  /* Do the transfer, and fill c_str with the result */
  status = curl_easy_perform (c_handle->handle);
  if (status != CURLE_OK)
    {
      error_code = status;
      return (SCM_BOOL_F);
    }

  if (c_handle->transfers < INT_MAX)
      c_handle->transfers ++;

  if (scm_is_true (headerflag))
    return (scm_list_2 (header_sf.scm, body_sf.scm));

  return (body_sf.scm);
}

/* This callback function catches the data passed by libcurl and sends
   it back as a scheme string */
static size_t
write_callback (void *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t length1, length2;
  SCM data1, data2;
  struct scm_flag *sf;

  sf = (struct scm_flag *) userdata;
  data1 = sf->scm;
  if (sf->flag)
    length1 = scm_c_bytevector_length (data1);
  else
    length1 = scm_c_string_length (data1);

  length2 = size * nmemb;

  /* printf ("In write_callback\n"); */
  if (sf->flag)
    {
      data2 = scm_c_make_bytevector (length1 + length2);
      memcpy (SCM_BYTEVECTOR_CONTENTS (data2),
              SCM_BYTEVECTOR_CONTENTS (data1),
              length1);
      memcpy (SCM_BYTEVECTOR_CONTENTS (data2) + length1,
              ptr,
              length2);
    }
  else
    {
      data2 = scm_c_make_string (length1 + length2, SCM_MAKE_CHAR('\0'));
      for (size_t i = 0; i < length1; i ++)
        {
          scm_c_string_set_x (data2, i, scm_c_string_ref (data1, i));
        }
      for (size_t i = 0; i < length2; i ++)
        {
          scm_c_string_set_x (data2, i + length1,
                              SCM_MAKE_CHAR (((char *)ptr)[i]));
        }
    }
  sf->scm = data2;

  return length2;
}

/* This callback function sends some data from a port over to curl. */
static size_t
read_callback (void *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t length1;
  SCM port;

  port = PTR2SCM(userdata);
  length1 = scm_c_read (port, ptr, size * nmemb);
  return length1;
}


SCM DLL_PUBLIC
cl_easy_reset (SCM handle)
{
  handle_post_t *c_handle;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-easy-reset");

  c_handle = _scm_to_handle (handle);

  curl_easy_reset (c_handle->handle);

  return SCM_UNSPECIFIED;
}

SCM DLL_PUBLIC
cl_easy_cleanup (SCM handle)
{
  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-easy-cleanup");

  gc_free_handle (handle);

  return SCM_UNSPECIFIED;
}

SCM DLL_PUBLIC
cl_error_string ()
{
  return (scm_from_locale_string (error_string));
}

SCM DLL_PUBLIC
cl_error_code ()
{
  return scm_from_int ((int) error_code);
}

static void
print_slist (struct curl_slist *sl)
{
  struct curl_slist *p = sl;
  int i = 0;
  while (p != NULL)
    {
      fprintf (stderr, "\t\t%d: \"%s\"\n", i, p->data);
      i++;
      p = p->next;
    }
}

static void
print_mem (char *data, long len)
{
  long print_len = len;
  if (print_len > 40)
    print_len = 40;
  if (data == NULL || len == 0)
    {
      fprintf (stderr, "(nil)");
      return;
    }
  for (long i = 0; i < print_len; i ++)
    {
      if (isprint (data[i]))
        fprintf (stderr, "%c", data[i]);
      else if (data[i] + 32 < 256)
        fprintf (stderr, "^%c", data[i] + 32);
      else
        fprintf (stderr, "?");
    }
  if (print_len != len)
    fprintf (stderr, "...");
}

static size_t
xstrlen (const char *s)
{
  if (s == NULL)
    return 0;
  return strlen (s);
}

static void
print_httppost (struct curl_httppost *hp)
{
  struct curl_httppost *p = hp;
  int i = 0;
  while (p != NULL)
    {
      fprintf (stderr, "\t\t%d: name: ", i);
      print_mem (p->name, p->namelength);
      fprintf (stderr, "\n\t\t   contents: ");
      print_mem (p->contents, p->contentslength);
      fprintf (stderr, "\n\t\t   buffer: ");
      print_mem (p->buffer, p->bufferlength);
      fprintf (stderr, "\n\t\t   contenttype: ");
      print_mem (p->contenttype, xstrlen (p->contenttype));
      fprintf (stderr, "\n\t\t   contentheader: ");
      print_slist (p->contentheader);
      fprintf (stderr, "\n\t\t   showfilename: ");
      print_mem (p->showfilename, xstrlen (p->showfilename));
      fprintf (stderr, "\n\t\t   flags: 0x%lx", p->flags);
      fprintf (stderr, "\n");
      i++;
      p = p->next;
    }
}


SCM DLL_PUBLIC
cl_dump_handle (SCM handle)
{
  handle_post_t *hp;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-dump-handle");

  hp = _scm_to_handle (handle);
  fprintf (stderr, "<#handle %p>\n", hp);
  fprintf (stderr, "\t        handle %p\n", hp->handle);
  fprintf (stderr, "\t    postfields %p\n", hp->postfields);
  fprintf (stderr, "\t postfieldsize %zu\n", hp->postfieldsize);
  print_mem (hp->postfields, hp->postfieldsize);
  fprintf (stderr, "\t      httppost %p\n", hp->httppost);
  print_httppost (hp->httppost);
  fprintf (stderr, "\t    httpheader %p\n", hp->httpheader);
  print_slist (hp->httpheader);
  fprintf (stderr, "\thttp200aliases %p\n", hp->http200aliases);
  print_slist (hp->http200aliases);
  fprintf (stderr, "\t     mail_rcpt %p\n", hp->mail_rcpt);
  print_slist (hp->mail_rcpt);
  fprintf (stderr, "\t         quote %p\n", hp->quote);
  print_slist (hp->quote);
  fprintf (stderr, "\t     postquote %p\n", hp->postquote);
  print_slist (hp->postquote);
  fprintf (stderr, "\t      prequote %p\n", hp->prequote);
  print_slist (hp->prequote);
  fprintf (stderr, "\t       resolve %p\n", hp->resolve);
  print_slist (hp->resolve);
  fprintf (stderr, "\t telnetoptions %p\n", hp->telnetoptions);
  print_slist (hp->telnetoptions);
  fflush (stderr);

  return SCM_UNDEFINED;
}

void
cl_init_func ()
{
  static int first = 1;

  if (first)
    {
      scm_c_define_gsubr ("%curl-easy-init", 0, 0, 0, cl_easy_init);
      scm_c_define_gsubr ("%curl-easy-getinfo", 2, 0, 0, cl_easy_getinfo);
      scm_c_define_gsubr ("%curl-easy-setopt", 4, 0, 0, cl_easy_setopt);
      scm_c_define_gsubr ("%curl-easy-perform", 3, 0, 0, cl_easy_perform);
      scm_c_define_gsubr ("%curl-easy-cleanup", 1, 0, 0, cl_easy_cleanup);
      scm_c_define_gsubr ("%curl-easy-reset", 1, 0, 0, cl_easy_reset);
      scm_c_define_gsubr ("%curl-error-string", 0, 0, 0, cl_error_string);
      scm_c_define_gsubr ("%curl-error-code", 0, 0, 0, cl_error_code);
      scm_c_define_gsubr ("%curl-dump-handle", 1, 0, 0, cl_dump_handle);

      first = 0;
    }
}
