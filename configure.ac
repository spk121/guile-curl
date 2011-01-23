# Process this file with autoconf to produce a configure script

##################
# Package info
AC_PREREQ([2.66])
AC_INIT([curl],[0.0],[spk121@yahoo.com])
AC_CONFIG_AUX_DIR([build_aux])
AM_INIT_AUTOMAKE([gnu std-options -Wall])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

##################
# Set some version constants
LIBGUILE_CURL_MAJOR=0
LIBGUILE_CURL_INTERFACE="0:0:0"
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
GUILE_PROGS
GUILE_FLAGS
GUILE_SITE_DIR
GUILE_CFLAGS
dnl AC_CHECK_LIB([guile-2.0], 
dnl              [scm_boot_guile], ,
dnl              [AC_MSG_ERROR([The guile library was not found])])

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
                build_aux/Makefile])

AC_OUTPUT