#ifndef CL_CONST_H
#define CL_CONST_H

#ifdef DLL_EXPORT
#define API __attribute__ ((dllexport, cdecl))
#else
#define API
#endif

void cl_init_const (void) API;

#endif
