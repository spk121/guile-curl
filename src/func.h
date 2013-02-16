#ifndef CL_FUNC_H
#define CL_FUNC_H

#include <libguile.h>
#include <curl/curl.h>
#include "visibility.h"

SCM DLL_PUBLIC cl_easy_init (void);
SCM DLL_PUBLIC cl_easy_getopt (SCM handle, SCM label, SCM val);
SCM DLL_PUBLIC cl_easy_perform (SCM handle, SCM bvflag);
SCM DLL_PUBLIC cl_easy_cleanup (SCM handle);
SCM DLL_PUBLIC cl_easy_reset (SCM handle);
SCM DLL_PUBLIC cl_error_string (void);
SCM DLL_PUBLIC cl_error_code (void);
SCM DLL_PUBLIC cl_dump_handle (SCM h);

void cl_init_func (void);

#endif
