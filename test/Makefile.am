# Copyright (C) 2011, 2013 Michael L. Gran

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TESTS_ENVIRONMENT = LIBGUILE_CURL_MAJOR=$(LIBGUILE_CURL_MAJOR)

################################################################
# Scheme-based tests

TEST_EXTENSIONS = .scm
SCM_LOG_COMPILER = ${top_builddir}/libtool \
    -dlopen ${top_builddir}/src/libguile-curl.la \
    --mode=execute guile
AM_SCM_LOG_FLAGS = -L ${top_srcdir}/src -L ${top_srcdir}/test/. -s

TESTS = setopt.scm \
	stress.scm

################################################################
# Compiled tests

LOG_COMPILER = ${top_builddir}/libtool \
    -dlopen ${top_builddir}/src/libguile-curl.la \
    --mode=execute

check_PROGRAMS = check_type

check_type_SOURCES = check_type.c
AM_CPPFLAGS = \
	$(CURL_CFLAGS) \
	$(GUILE_CFLAGS) \
	-lffi

AM_LDFLAGS = \
	-L../src -lguile-curl \
	$(CURL_LDFLAGS) \
	$(GUILE_LDFLAGS)

TESTS += check_type

################################################################

EXTRA_DIST = setopt.scm \
	stress.scm \
	lib.scm \
	do_gdb \
	setopt.scm.in

BUILT_SOURCES = setopt.scm
CLEANFILES = setopt.scm

setopt.scm: setopt.scm.in
	if test "x$(RNRS_BYTEVECTORS)" = "xno" ; then \
	  cat setopt.scm.in | sed -e 's|;; RNRS_BYTEVECTORS_BEGIN|#!|' -e 's|;; RNRS_BYTEVECTORS_END|!#|' > $@ ; \
	else \
	  cat setopt.scm.in > $@ ; \
	fi
