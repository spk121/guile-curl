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

## Process this file with automake to produce Makefile.in
TESTS_ENVIRONMENT = LIBGUILE_CURL_MAJOR=$(LIBGUILE_CURL_MAJOR)
TESTS_ENVIRONMENT += ../libtool -dlopen ../src/libguile-curl.la \
	--mode=execute \
	guile -L $(top_srcdir)/src -L $(top_srcdir)/test/. -s

TESTS = setopt.scm \
	stress.scm

EXTRA_DIST = $(TESTS) \
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
