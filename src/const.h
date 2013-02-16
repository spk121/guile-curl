/* Constants for guile-curl

   Copyright (c) 2013 Michael L. Gran

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
#ifndef CL_CONST_H
#define CL_CONST_H

#ifdef DLL_EXPORT
#define API __attribute__ ((dllexport, cdecl))
#else
#define API
#endif

void cl_init_const (void) API;

#endif
