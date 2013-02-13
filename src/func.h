#ifndef CL_FUNC_H
#define CL_FUNC_H

#include <libguile.h>
#include <curl/curl.h>

#ifdef DLL_EXPORT
#define API __attribute__ ((dllexport, cdecl))
#else
#define API
#endif

SCM cl_easy_init (void) API;
SCM cl_easy_getopt (SCM handle, SCM label, SCM val) API;
SCM cl_easy_perform (SCM handle, SCM bvflag) API;
SCM cl_easy_cleanup (SCM handle) API;
SCM cl_easy_reset (SCM handle) API;
// SCM cl_global_cleanup (void) API;
// SCM cl_global_init (void) API;
SCM cl_error_string (void) API;
SCM cl_error_code (void) API;

void cl_init_func (void) API;

#endif
