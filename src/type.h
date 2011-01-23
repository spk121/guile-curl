#ifndef CL_TYPE_H
#define CL_TYPE_H

#include <curses.h>
#include <curl/curl.h>

#ifdef DLL_EXPORT
#define API __attribute__ ((dllexport, cdecl))
#else
#define API
#endif

void cl_init_type (void) API;

int _scm_is_handle (SCM x);
CURL *_scm_to_handle (SCM x);
SCM _scm_from_handle (CURL *x);

SCM cl_is_handle_p (SCM x) API;

#endif
