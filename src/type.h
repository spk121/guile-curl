#ifndef CL_TYPE_H
#define CL_TYPE_H

#include <curses.h>
#include <curl/curl.h>

#ifdef DLL_EXPORT
#define API __attribute__ ((dllexport, cdecl))
#else
#define API
#endif

typedef struct handle_post_tag
{
  CURL *handle;
  uint8_t *postfields;
  // size_t postfieldsize;
  // curl_off_t postfieldsize_large;
  struct curl_httppost *httppost;
  struct curl_slist *httpheader;
  struct curl_slist *http200aliases;
  struct curl_slist *mail_rcpt;
  struct curl_slist *quote;
  struct curl_slist *postquote;
  struct curl_slist *prequote;
  struct curl_slist *resolve;
  struct curl_slist *telnetoptions;
} handle_post_t;

void cl_init_type (void) API;

int _scm_is_handle (SCM x);
handle_post_t *_scm_to_handle (SCM x);
SCM _scm_from_handle (handle_post_t *x);

SCM cl_is_handle_p (SCM x) API;

int _scm_can_convert_to_slist (SCM x);
struct curl_slist *_scm_convert_to_slist (SCM x);
int _scm_can_convert_to_byte_data (SCM x);
uint8_t *_scm_convert_to_byte_data (SCM x, size_t *len);
int _scm_can_convert_to_httppost (SCM x);
struct curl_httppost *_scm_convert_to_httppost (SCM x);

#endif
