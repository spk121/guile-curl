#include <config.h>
#include <libguile.h>
#include <curl/curl.h>
#include "const.h"
#include "type.h"
#include "func.h"

#ifdef DLL_EXPORT
#define API __attribute__ ((dllexport, cdecl))
#else
#define API
#endif

void cl_init (void) API;

void
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
