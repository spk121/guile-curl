# Process this file with autoconf to produce a configure script
# Process this file with autoconf to produce a configure script

# Copyright 20011, 2013 Michael L Gran

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
AC_PREREQ([2.61])
AC_INIT([guile-curl],[0.2],[spk121@yahoo.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([gnu std-options -Wall])
gl_EARLY
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
gl_INIT

##################
# Set some version constants
LIBGUILE_CURL_MAJOR=0
LIBGUILE_CURL_INTERFACE="0:1:0"
AC_SUBST(LIBGUILE_CURL_MAJOR)
AC_SUBST(LIBGUILE_CURL_INTERFACE)

##################
# Check for Programs
AC_PROG_CC
AC_CANONICAL_HOST
AC_PROG_INSTALL
LT_INIT
dnl LT_INIT([dlopen disable-fast-install win32-dll])
AC_PROG_MKDIR_P


##################
# Initial guess of platform specific options
case $host in
     *-*-cygwin)
	[enable_etex_fix=yes] ;;
     *-*-linux*)
	[enable_etex_fix=no] ;;
     *)
	[enable_etex_fix=no] ;;
esac

AM_CONDITIONAL([ETEX_FIX], [test "$enable_etex_fix" = yes])

##################
# Check for libraries

# Guile gets added to LIBS here
AC_CHECK_LIB([curl],
             [curl_global_init], ,
             [AC_MSG_ERROR([The curl library was not found])])

AC_ARG_WITH([guilesitedir],
  [use the specified installation path for Guile modules],
  [case "x$withval" in
    xyes|xno) guilesitedir="";;
    *)        guilesitedir="$withval";;
   esac],
  [guilesiteir=""])

GUILE_PROGS
GUILE_FLAGS
GUILE_SITE_DIR
GUILE_FLAGS

pkgdatadir="$datadir/$PACKAGE_NAME"
if test "x$guilesitedir" = "x"; then
  guilesitedir="$pkgdatadir"
fi
AC_SUBST([guilesitedir])

AC_ARG_WITH([guileextensiondir],
  [use the specified installation path for Guile binary shared libraries],
  [case "x$withval" in
    xyes|xno) guileextensiondir="";;
    *)        guileextensiondir="$withval";;
   esac],
  [guileextensiondir=""])

if test "x$guileextensiondir" = "x"; then
  guileextensiondir="$libdir"
fi
AC_SUBST([guileextensiondir])

GUILE_EXTENSION_DIR=`pkg-config guile-2.0 --variable=extensiondir`
if test "x$GUILE_EXTENSION_DIR" = "x"; then
GUILE_EXTENSION_DIR=`guile-config info libdir`
fi
AC_SUBST([GUILE_EXTENSION_DIR])


##################
# Check for header files

AC_CHECK_HEADERS([curl/curl.h], ,
                 [AC_MSG_ERROR([<curl.h> was not found])])
dnl AC_CHECK_HEADERS([libguile.h], ,
dnl                  [AC_MSG_ERROR([<libguile.h> was not found])])


##################
# Check for types, structures, compiler characteristics
AC_PROG_CC_C99

##################
# Check for library functions

# Later something should be done with this
# AC_FUNC_MALLOC

##################
# Check for system services

##################
#  Output
AC_CONFIG_FILES([Makefile
                src/Makefile
                doc/Makefile
                test/Makefile
                m4/Makefile
                build-aux/Makefile])

AC_OUTPUT

if test "$guilesitedir" != "$GUILE_SITE"; then
   # Guile has a different prefix than this module
   AC_MSG_WARN([]
   [The Guile modules will be installed in '${guilesitedir}'.]
   [You should probably re-run `configure' with]
   [`--with-guilesitedir=$GUILE_SITE']
   [Otherwise, you will have to adjust the `GUILE_LOAD_PATH' environment]
   [variable.])
fi

if test "x$GUILE_EXTENSION_DIR" != "x"; then
if test "$guileextensiondir" != "$GUILE_EXTENSION_DIR"; then
   AC_MSG_WARN([]
   [The Guile extensions will be installed in ${guileextensiondir}.]
   [You should consider re-running `configure' with]
   [`--with-guileextensiondir=$GUILE_EXTENSION_DIR'])
fi
fi
