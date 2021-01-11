# Process this file with autoconf to produce a configure script

# Copyright 2001, 2013, 2014, 2016, 2017, 2019, 2021 Michael L Gran

# This file is part of guile-curl.

# guile-curl is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# guile-curl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with guile-curl.  If not, see <http://www.gnu.org/licenses/>.

##################
# Package info
AC_PREREQ([2.64])
AC_INIT([guile_curl],[0.8],[spk121@yahoo.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([no-dependencies subdir-objects -Wall foreign])
PKG_PROG_PKG_CONFIG
AC_PROG_CC_C99
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

##################
# Set some version constants
LIBGUILE_CURL_MAJOR=2
LIBGUILE_CURL_INTERFACE="2:1:0"
AC_SUBST(LIBGUILE_CURL_MAJOR)
AC_SUBST(LIBGUILE_CURL_INTERFACE)

##################
# Check for Programs
AC_CANONICAL_HOST
AC_PROG_INSTALL
AM_PROG_AR
LT_INIT([disable-fast-install win32-dll disable-static shared])
AC_PROG_MKDIR_P


##################
# Initial guess of platform specific options

AS_CASE([$host_os],
        [mingw*], [building_dll=yes;dll_version_info=no],
        [cygwin*], [building_dll=yes;dll_version_info=yes],
        [pw32*], [building_dll=yes;dll_version_info=yes],
        [cegcc*], [building_dll=yes;dll_version_info=yes],
        [building_dll=no;dll_version_info=yes])

AM_CONDITIONAL([BUILDING_DLL], [test "$building_dll" = yes])
AM_CONDITIONAL([DLL_VERSION_INFO], [test "$dll_version_info" = yes])

##################
# Check for libraries
PKG_CHECK_MODULES(CURL, [libcurl])
GUILE_PKG([3.0 2.2])
GUILE_PROGS
GUILE_FLAGS
GUILE_SITE_DIR

dnl Installation directories for .scm and .go files.
guilemoduledir="${datarootdir}/guile/site/$GUILE_EFFECTIVE_VERSION"
guileobjectdir="${libdir}/guile/$GUILE_EFFECTIVE_VERSION/site-ccache"
guileextensiondir=${libdir}/guile/$GUILE_EFFECTIVE_VERSION/extensions
AC_SUBST([guilemoduledir])
AC_SUBST([guileobjectdir])
AC_SUBST([guileextensiondir])

AC_ARG_ENABLE([guild],
              [AS_HELP_STRING([--disable-guild],
                              [don't compile guile sources])],
              [AS_CASE(["${enableval}"],
                       [yes], [guild=true],
                       [no], [guild=false],
                       [AC_MSG_ERROR([bad value ${enableval} for --enable-guild])])],
              [guild=true])
AM_CONDITIONAL([GUILD], [test x$guild = xtrue])


##################
# Check for header files

tmp_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $CURL_CFLAGS $GUILE_CFLAGS"
AC_CHECK_HEADER([curl/curl.h], ,
                [AC_MSG_ERROR([<curl.h> was not found])])
AC_CHECK_HEADERS([libguile.h], ,
                 [AC_MSG_ERROR([<libguile.h> was not found])])
CPPFLAGS="$tmp_CPPFLAGS"

##################
# Check for types, structures, compiler characteristics

m4_define([compiler_flags_common],[ dnl
 -D_FORTIFY_SOURCE=2 dnl
 -DSCM_DEBUG_TYPING_STRICTNESS=2 dnl
 -O2 dnl
 -pipe dnl
 -g dnl
 -grecord-gcc-switches dnl
 -Wall dnl
 -Werror=format-security dnl
 -Werror=implicit-function-declaration dnl
 -Werror=shadow dnl
 -fno-omit-frame-pointer dnl
 -fasynchronous-unwind-tables dnl
 -fexceptions dnl
 -fvar-tracking dnl
 -fstack-protector-strong dnl
 -fstack-clash-protection dnl
 -fcf-protection dnl
 -fplugin=annobin dnl
 -Wl,--dynamic-list-data dnl
 -Wl,-z,defs dnl
 -Wl,-z,now dnl
 -Wl,-z,relro dnl
 -Wsuggest-attribute=format dnl
 -Wsuggest-attribute=malloc dnl
 -Wsuggest-attribute=noreturn dnl
 -Wsuggest-attribute=pure dnl
 -fdiagnostics-color=auto dnl
])

AC_ARG_ENABLE([hardening],
              [AS_HELP_STRING([--enable-hardening],
                              [compile with common hardening flags])])
AS_IF([test "x$enable_hardening" = xyes],
      [CC_CHECK_CFLAGS_APPEND([compiler_flags_common])])

##################
# Check for library functions

##################
# Check for system services

##################
#  Output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

if test "$guilemoduledir" != "$GUILE_SITE"; then
   # Guile has a different prefix than this module
   AC_MSG_WARN([]
   [The Guile modules will be installed in '${guilemoduledir}'.]
   [This Guile expects them in ${GUILE_SITE}.]
   [If these differ, you may have to adjust the `GUILE_LOAD_PATH' environment]
   [variable.])
fi

if test "x$GUILE_EXTENSION" != "x"; then
if test "$guileextensiondir" != "$GUILE_EXTENSION"; then
   AC_MSG_WARN([]
   [The Guile extensions will be installed in ${guileextensiondir}.]
   [This Guile expects them in ${GUILE_EXTENSION}.])
fi
fi
