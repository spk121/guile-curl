/* Constants for guile-curl

   Copyright (c) 2011, 2013 Michael L. Gran

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
#ifndef CL_FUNC_H
#define CL_FUNC_H

#include <libguile.h>
#include <curl/curl.h>
#include "visibility.h"

SCM DLL_PUBLIC cl_easy_init (void);
SCM DLL_PUBLIC cl_easy_getopt (SCM handle, SCM label, SCM val);
SCM DLL_PUBLIC cl_easy_setopt (SCM handle, SCM option, SCM param, SCM big);
SCM DLL_PUBLIC cl_easy_perform (SCM handle, SCM bvflag);
SCM DLL_PUBLIC cl_easy_cleanup (SCM handle);
SCM DLL_PUBLIC cl_easy_reset (SCM handle);
SCM DLL_PUBLIC cl_error_string (void);
SCM DLL_PUBLIC cl_error_code (void);
SCM DLL_PUBLIC cl_dump_handle (SCM h);

void cl_init_func (void);

#endif
