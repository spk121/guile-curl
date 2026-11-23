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

/* The error state reported by curl-error-code and curl-error-string.
   It is per thread, so that one thread's failure does not overwrite
   the error another thread is about to read.  */
static SCM_THREAD_LOCAL CURLcode error_code = CURLE_OK;
static SCM_THREAD_LOCAL char error_string[CURL_ERROR_SIZE+1] = "";

/* Received data (body or headers) is collected here by write_callback
   and converted to a Scheme object once, after the transfer.  */
struct recv_buffer
{
  char *data;
  size_t len;
  size_t cap;
};

/* Errors must not unwind through libcurl's stack frames: curl forbids
   longjmp out of its callbacks, and doing so leaves the easy handle in
   the middle of a transfer.  So read_callback runs its Scheme code under
   a catch, stores any exception, and tells curl to abort.  */
static SCM
store_exception_handler (void *data, SCM key, SCM args)
{
  *(SCM *) data = scm_cons (key, args);
  return SCM_BOOL_F;
}

/* Options every new or reset handle starts with.

   With a synchronous DNS resolver, libcurl implements DNS timeouts with
   SIGALRM and siglongjmp, which is unsafe in a multithreaded program,
   and a Guile process always has several threads.  libcurl's
   documentation says such programs must set CURLOPT_NOSIGNAL, so do so
   by default in that case.  With an asynchronous resolver (threaded or
   c-ares) libcurl does not use SIGALRM, and the default is left alone.
   Either way, (curl-easy-setopt h 'nosignal ...) overrides it.  */
static void
set_default_options (CURL *handle)
{
  if (!(curl_version_info (CURLVERSION_NOW)->features & CURL_VERSION_ASYNCHDNS))
    curl_easy_setopt (handle, CURLOPT_NOSIGNAL, 1L);
}

SCM DLL_PUBLIC cl_easy_init ()
{
  CURL *handle;
  handle_post_t *hp;

  handle = curl_easy_init ();

  if (handle == NULL)
    {
      scm_misc_error ("curl-easy-init",
                      "initialization failure",
                      SCM_EOL);
    }

  /* Allocated from the GC heap, not with malloc, so that the GC scans
     its SCM fields (such as readport).  The smob keeps it alive.  */
  hp = scm_gc_malloc (sizeof (handle_post_t), "curl handle");
  memset (hp, 0, sizeof (handle_post_t));
  hp->handle = handle;
  set_default_options (handle);
  hp->readport = SCM_BOOL_F;
  hp->pending_exception = SCM_BOOL_F;

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
          curl_easy_setopt (c_handle->handle, CURLOPT_POSTFIELDSIZE_LARGE,
                            (curl_off_t) len);
          c_handle->postfieldsize = len;
          code = curl_easy_setopt (c_handle->handle, CURLOPT_POSTFIELDS, (char *) m);
        }
      else
        scm_misc_error ("curl-easy-setopt",
                        "CURLOPT_POSTFIELDS requires 8-bit string or bytevector data",
                        SCM_EOL);
    }
  else if (c_option == CURLOPT_COPYPOSTFIELDS)
    {
      if (_scm_can_convert_to_byte_data (param))
        {
          size_t len;
          uint8_t *m = _scm_convert_to_byte_data (param, &len);
          /* With the size set first, curl copies exactly LEN bytes, so
             binary data with NULs works.  */
          curl_easy_setopt (c_handle->handle, CURLOPT_POSTFIELDSIZE_LARGE,
                            (curl_off_t) len);
          code = curl_easy_setopt (c_handle->handle, CURLOPT_COPYPOSTFIELDS,
                                   m ? (char *) m : "");
          free (m);
          /* Curl no longer uses any buffer kept for CURLOPT_POSTFIELDS.  */
          free (c_handle->postfields);
          c_handle->postfields = NULL;
          c_handle->postfieldsize = 0;
        }
      else
        scm_misc_error ("curl-easy-setopt",
                        "CURLOPT_COPYPOSTFIELDS requires 8-bit string or bytevector data",
                        SCM_EOL);
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
        scm_misc_error ("curl-easy-setopt",
                        "CURLOPT_HTTPHEADER requires a list of strings",
                        SCM_EOL);
    }
  else if (c_option == CURLOPT_HTTPPOST)
    {
      /* Checked by option rather than by argument type, because an
         empty form, '(), would also pass as a list of strings.  */
      if (_scm_can_convert_to_httppost (param))
        {
          struct curl_httppost *p;
          p = _scm_convert_to_httppost (param);
          curl_formfree (c_handle->httppost);
          c_handle->httppost = p;
          /* An empty form gives P == NULL, which clears the form.  */
          code = curl_easy_setopt (c_handle->handle, CURLOPT_HTTPPOST, p);
        }
      else
        scm_misc_error ("curl-easy-setopt",
                        "CURLOPT_HTTPPOST requires a list of form fields",
                        SCM_EOL);
    }
  else if (scm_is_integer (param))
    {
      if (scm_is_true (big))
        code = curl_easy_setopt (c_handle->handle, c_option,
                                 (curl_off_t) scm_to_int64 (param));
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
          // Bad slist option.  Nothing took ownership of SL.
          curl_slist_free_all (sl);
          ok = 0;
        }
      if (ok)
        code = curl_easy_setopt (c_handle->handle, c_option, sl);
      else
        scm_misc_error ("curl-easy-setopt",
                        "this option does not accept a list of strings",
                        SCM_EOL);
    }
  else if (scm_is_true (scm_input_port_p (param)))
    {
      if (c_option == CURLOPT_READDATA)
        {
          c_handle->readport = param;
          curl_easy_setopt (c_handle->handle, CURLOPT_READFUNCTION, read_callback);
          code = curl_easy_setopt (c_handle->handle, CURLOPT_READDATA, c_handle);
        }
      else
        scm_misc_error ("curl-easy-setopt",
                        "this option does not accept a port",
                        SCM_EOL);
    }
  else
    scm_misc_error ("curl-easy-setopt",
                    "unimplemented option type",
                    SCM_EOL);
  if (code != CURLE_OK)
    {
      error_code = code;
      scm_misc_error ("curl-easy-setopt", "~A",
                      scm_list_1 (scm_from_locale_string
                                  (curl_easy_strerror (code))));
    }

  return SCM_UNSPECIFIED;
}

static void
free_slist (void *sl)
{
  curl_slist_free_all ((struct curl_slist *) sl);
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
        {
#ifdef _WIN32
          /* SOCKET is a pointer-sized unsigned integer (64 bits on
             64-bit Windows).  */
          return scm_from_uintptr_t ((uintptr_t) c_sockfd);
#else
          return scm_from_int (c_sockfd);
#endif
        }
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
#ifdef CURLINFO_EARLYDATA_SENT_T
           || (c_option == CURLINFO_EARLYDATA_SENT_T)
#endif
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
           || (c_option == CURLINFO_SPEED_UPLOAD_T)
           || (c_option == CURLINFO_STARTTRANSFER_TIME_T)
           || (c_option == CURLINFO_TOTAL_TIME_T)
           )
    {
      curl_off_t c_val;
      code = curl_easy_getinfo (c_handle->handle, c_option, &c_val);
      if (code == CURLE_OK)
        return scm_from_int64 ((int64_t) c_val);
      else
        {
          error_code = code;
          return SCM_BOOL_F;
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

          /* scm_from_utf8_string throws on invalid UTF-8, so free the
             list on any exit, not just the normal one.  */
          scm_dynwind_begin (0);
          scm_dynwind_unwind_handler (free_slist, c_cookies,
                                      SCM_F_WIND_EXPLICITLY);
          while (c_each)
            {
              lst = scm_cons (scm_from_utf8_string (c_each->data), lst);
              c_each = c_each->next;
            }
          scm_dynwind_end ();
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
    scm_misc_error ("curl-easy-getinfo",
                    "unimplemented option type",
                    SCM_EOL);
  return SCM_BOOL_F;
}


static void
free_recv_buffer (void *data)
{
  struct recv_buffer *buf = data;
  free (buf->data);
  buf->data = NULL;
  buf->len = buf->cap = 0;
}

/* Received bytes become a string with one character per byte
   (ISO-8859-1), or a bytevector.  */
static SCM
recv_buffer_to_scm (struct recv_buffer *buf, int bytevector)
{
  if (bytevector)
    {
      SCM bv = scm_c_make_bytevector (buf->len);
      if (buf->len > 0)
        memcpy (SCM_BYTEVECTOR_CONTENTS (bv), buf->data, buf->len);
      return bv;
    }
  return scm_from_latin1_stringn (buf->len > 0 ? buf->data : "", buf->len);
}

SCM DLL_PUBLIC
cl_easy_perform (SCM handle, SCM bvflag, SCM headerflag)
{
  handle_post_t *c_handle;
  CURLcode status;
  struct recv_buffer body = { NULL, 0, 0 };
  struct recv_buffer header = { NULL, 0, 0 };
  SCM result;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-easy-perform");

  c_handle = _scm_to_handle (handle);
  c_handle->pending_exception = SCM_BOOL_F;

  /* The buffers are freed however this procedure exits.  */
  scm_dynwind_begin (0);
  scm_dynwind_unwind_handler (free_recv_buffer, &body, SCM_F_WIND_EXPLICITLY);
  scm_dynwind_unwind_handler (free_recv_buffer, &header,
                              SCM_F_WIND_EXPLICITLY);

  if (scm_is_true (headerflag))
  {
    curl_easy_setopt (c_handle->handle, CURLOPT_HEADERFUNCTION, write_callback);
    curl_easy_setopt (c_handle->handle, CURLOPT_HEADERDATA, &header);
  }
  else
  {
    /* Clear any header callback left over from a previous call, whose
       HEADERDATA would point into that call's (now dead) stack frame.  */
    curl_easy_setopt (c_handle->handle, CURLOPT_HEADERFUNCTION, NULL);
    curl_easy_setopt (c_handle->handle, CURLOPT_HEADERDATA, NULL);
  }

  curl_easy_setopt (c_handle->handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt (c_handle->handle, CURLOPT_WRITEDATA, &body);
  curl_easy_setopt (c_handle->handle, CURLOPT_ERRORBUFFER, error_string);

  status = curl_easy_perform (c_handle->handle);

  /* ERRORBUFFER points at this thread's error_string.  Clear it, so that
     curl never writes there from a later call made on another thread
     (by which time this thread may have exited).  */
  curl_easy_setopt (c_handle->handle, CURLOPT_ERRORBUFFER, NULL);

  /* Count failed transfers too: getinfo is still meaningful after one
     (e.g. CURLINFO_OS_ERRNO, CURLINFO_RESPONSE_CODE).  */
  if (c_handle->transfers < INT_MAX)
    c_handle->transfers ++;

  /* Rethrow any Scheme error caught inside read_callback, now that curl
     has returned normally.  */
  if (scm_is_true (c_handle->pending_exception))
    {
      SCM exc = c_handle->pending_exception;
      c_handle->pending_exception = SCM_BOOL_F;
      error_code = status;
      scm_throw (scm_car (exc), scm_cdr (exc));
    }

  if (status != CURLE_OK)
    {
      error_code = status;
      result = SCM_BOOL_F;
    }
  else if (scm_is_true (headerflag))
    result = scm_list_2 (recv_buffer_to_scm (&header, 0),
                         recv_buffer_to_scm (&body, scm_is_true (bvflag)));
  else
    result = recv_buffer_to_scm (&body, scm_is_true (bvflag));

  scm_dynwind_end ();
  return result;
}

/* This callback function collects the data passed by libcurl.  It runs
   no Scheme code, so it cannot throw.  */
static size_t
write_callback (void *ptr, size_t size, size_t nmemb, void *userdata)
{
  struct recv_buffer *buf = (struct recv_buffer *) userdata;
  size_t length = size * nmemb;

  if (length > SIZE_MAX - buf->len)
    return 0;                   /* Makes curl abort with CURLE_WRITE_ERROR.  */
  if (buf->len + length > buf->cap)
    {
      /* Grow geometrically, so the total copying stays linear.  */
      size_t cap = buf->cap ? buf->cap : 16384;
      char *data;
      while (cap < buf->len + length)
        cap = (cap > SIZE_MAX / 2) ? buf->len + length : cap * 2;
      data = realloc (buf->data, cap);
      if (data == NULL)
        return 0;
      buf->data = data;
      buf->cap = cap;
    }
  memcpy (buf->data + buf->len, ptr, length);
  buf->len += length;
  return length;
}

struct read_args
{
  SCM port;
  void *ptr;
  size_t length;
};

static SCM
read_callback_body (void *data)
{
  struct read_args *args = data;
  return scm_from_size_t (scm_c_read (args->port, args->ptr, args->length));
}

/* This callback function sends some data from a port over to curl.  */
static size_t
read_callback (void *ptr, size_t size, size_t nmemb, void *userdata)
{
  handle_post_t *hp = (handle_post_t *) userdata;
  struct read_args args;
  SCM ret;

  args.port = hp->readport;
  args.ptr = ptr;
  args.length = size * nmemb;

  ret = scm_c_catch (SCM_BOOL_T,
                     read_callback_body, &args,
                     store_exception_handler, &hp->pending_exception,
                     NULL, NULL);
  if (scm_is_false (ret))
    return CURL_READFUNC_ABORT;
  return scm_to_size_t (ret);
}


SCM DLL_PUBLIC
cl_easy_reset (SCM handle)
{
  handle_post_t *c_handle;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-easy-reset");

  c_handle = _scm_to_handle (handle);

  curl_easy_reset (c_handle->handle);
  set_default_options (c_handle->handle);
  c_handle->readport = SCM_BOOL_F;
  c_handle->pending_exception = SCM_BOOL_F;

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
print_mem (const void *data, long len)
{
  const unsigned char *bytes = data;
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
      /* isprint needs an unsigned char value (or EOF).  */
      unsigned char c = bytes[i];
      if (isprint (c))
        fprintf (stderr, "%c", c);
      else if (c < 32)
        fprintf (stderr, "^%c", c + 64);   /* Caret notation: 1 is ^A.  */
      else if (c == 127)
        fprintf (stderr, "^?");
      else
        fprintf (stderr, "\\x%02x", c);
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

  return SCM_UNSPECIFIED;
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
