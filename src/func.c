#include <config.h>
#include <libguile.h>
#include <curl/curl.h>
#include "type.h"
#include "func.h"

static size_t write_callback (void *ptr, size_t size, size_t nmemb,
			      void *port);

#define SCM_FROM_CHAR(c) scm_integer_to_char(scm_from_char(c))
CURLcode error_code = CURLE_OK;
char error_string[CURL_ERROR_SIZE+1] = "";

struct scm_flag
{
  SCM scm;
  int flag;
};

SCM cl_easy_init ()
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
  memset (hp, sizeof (handle_post_t), 0);
  hp->handle = handle;

  return _scm_from_handle (hp);
}

SCM
cl_easy_setopt (SCM handle, SCM option, SCM param, SCM big)
{
  handle_post_t *c_handle;
  CURLoption c_option;
  CURLcode code = CURLE_UNSUPPORTED_PROTOCOL;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "curl-easy-setopt");
  SCM_ASSERT (scm_is_integer (option), option, SCM_ARG2, "curl-easy-setopt");

  c_handle = _scm_to_handle (handle);
  c_option = (CURLoption) scm_to_int (option);
  if (scm_is_integer (param))
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
      if (c_option == CURLOPT_HTTPHEADER)
	{
	  if (c_handle->httpheader)
	    curl_slist_free_all (c_handle->httpheader);
	  c_handle->httpheader = sl;
	}
      else if (c_option == CURLOPT_HTTP200ALIASES)
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
      code = curl_easy_setopt (c_handle->handle, c_option, sl);
    }
  else if (_scm_can_convert_to_byte_data (param))
    {
      if (c_option == CURLOPT_POSTFIELDS)
	{
	  size_t len;
	  uint8_t *m = _scm_convert_to_byte_data (param, &len);
	  if (c_handle->postfields)
	    free (c_handle->postfields);
	  c_handle->postfields = m;
	  curl_easy_setopt (c_handle->handle, CURLOPT_POSTFIELDS, len);
	  code = curl_easy_setopt (c_handle->handle, CURLOPT_POSTFIELDS, m);
	}
    }
  else if (_scm_can_convert_to_httppost (param))
    {
      if (c_option == CURLOPT_HTTPPOST)
	{
	  struct curl_httppost *p;
	  p = _scm_convert_to_httppost (param);
	  if (c_handle->httppost)
	    free (c_handle->httppost);
	  c_handle->httppost = p;
	  code = curl_easy_setopt (c_handle, CURLOPT_HTTPPOST, p);
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

SCM
cl_easy_perform (SCM handle, SCM bvflag)
{
  CURL *c_handle;
  SCM data;
  CURLcode status;
  struct scm_flag sf;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-easy-perform");

  c_handle = _scm_to_handle (handle);
  data = scm_c_make_bytevector (0);
  sf.scm = data;
  sf.flag = scm_is_true (bvflag);

  curl_easy_setopt (c_handle, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt (c_handle, CURLOPT_WRITEDATA, &sf);
  curl_easy_setopt (c_handle, CURLOPT_ERRORBUFFER, error_string);

  /* Do the transfer, and fill c_str with the result */
  status = curl_easy_perform (c_handle);
  if (status != CURLE_OK)
    {
      error_code = status;
      return (SCM_BOOL_F);
    }
  else
    {
      return (data);
    }
}

/* This callback function catches the data passed by libcurl and sends
   it back as a scheme string */
static size_t
write_callback (void *ptr, size_t size, size_t nmemb, void *port)
{
  size_t length1, length2;
  SCM data1, data2;
  struct scm_flag *sf;
  size_t i;

  sf = (struct scm_flag *) port;
  data1 = sf->scm;
#if GUILE_MAJOR_VERSION > 1
  if (sv->flag)
    length1 = scm_c_bytevector_length (data1);
  else
    length1 = scm_c_string_length (data1);
#else
  length1 = scm_c_string_length (data1);
#endif

  length2 = size * nmemb;

  /* printf ("In write_callback\n"); */
#if SCM_MAJOR_VERSION == 2
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
      data2 = scm_c_make_string (length1 + length2, SCM_FROM_CHAR('\0'));
      for (i = 0; i < length1; i ++)
	{
	  scm_c_string_set_x (data2, i, scm_c_string_ref (data1, i));
	}
      for (i = length1; i < length1 + length2; i ++);
      {
	scm_c_string_set_x (data2, i,
			    SCM_FROM_CHAR (((unsigned char *)ptr)[i - length1]));
      }
    }
#else
  data2 = scm_c_make_string (length1 + length2, SCM_FROM_CHAR('\0'));
  memcpy (SCM_STRING_CHARS (data2),
	  SCM_STRING_CHARS (data1),
	  length1);
  memcpy (SCM_STRING_CHARS (data2) + length1,
	  ptr,
	  length2);
#endif
  sf->scm = data2;

  return (length2);
}

SCM
cl_easy_reset (SCM handle)
{
  CURL *c_handle;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-easy-reset");

  c_handle = _scm_to_handle (handle);

  curl_easy_reset (c_handle);

  return SCM_UNSPECIFIED;
}

SCM
cl_easy_cleanup (SCM handle)
{
  CURL *c_handle;

  SCM_ASSERT (_scm_is_handle (handle), handle, SCM_ARG1, "%curl-easy-cleanup");

  c_handle = _scm_to_handle (handle);

  curl_easy_cleanup (c_handle);

  c_handle = (CURL *) NULL;

  return SCM_UNSPECIFIED;
}

SCM
cl_error_string ()
{
  return (scm_from_locale_string (error_string));
}

SCM
cl_error_code ()
{
  return scm_from_int ((int) error_code);
}

void
cl_init_func ()
{
  static int first = 1;

  if (first)
    {
      scm_c_define_gsubr ("%curl-easy-init", 0, 0, 0, cl_easy_init);
      scm_c_define_gsubr ("%curl-easy-setopt", 4, 0, 0, cl_easy_setopt);
      scm_c_define_gsubr ("%curl-easy-perform", 2, 0, 0, cl_easy_perform);
      scm_c_define_gsubr ("%curl-easy-cleanup", 1, 0, 0, cl_easy_cleanup);
      scm_c_define_gsubr ("%curl-easy-reset", 1, 0, 0, cl_easy_reset);
      scm_c_define_gsubr ("%curl-error-string", 0, 0, 0, cl_error_string);
      scm_c_define_gsubr ("%curl-error-code", 0, 0, 0, cl_error_code);

      first = 0;
    }
}
