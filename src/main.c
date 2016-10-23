/* guile-curl

   Copyright (c) 2011, 2013, 2014, 2016 Michael L. Gran

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
#include "const.h"
#include "type.h"
#include "func.h"
#include "visibility.h"

void DLL_PUBLIC cl_init (void);

void DLL_PUBLIC
cl_init ()
{
  CURLcode ret;

  ret = curl_global_init (CURL_GLOBAL_ALL);
  if (ret)
    scm_misc_error ("curl initialization", "initialization failed", SCM_EOL);
  else
    {
      cl_init_type ();
      cl_init_const ();
      cl_init_func ();

      atexit (curl_global_cleanup);
    }
}
