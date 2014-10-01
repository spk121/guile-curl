## Process this file with automake to produce Makefile.in

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

### MAIN TARGETS ###

guileextension_LTLIBRARIES = \
	libguile-curl.la

dist_guilesite_DATA = \
	curl.scm

####################

libguile_curl_la_CPPFLAGS = \
	$(CURL_CFLAGS) \
	$(GUILE_CFLAGS)
#	$(CFLAG_VISIBILITY)

#libguile_curl_la_CPPFLAGS += -Wall -Wextra -Wstrict-prototypes		\
#-Wmissing-prototypes -Wpointer-arith -Wreturn-type -Wcast-qual		\
#-Wswitch -Wshadow -Wcast-align -Wwrite-strings -Wchar-subscripts	\
#-Winline -Wnested-externs -Wredundant-decls -Wunused -Wformat=2

libguile_curl_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info $(LIBGUILE_CURL_INTERFACE) \
	$(CURL_LDFLAGS) \
	$(GUILE_LDFLAGS)

libguile_curl_la_SOURCES = \
	visibility.h \
	const.c const.h \
	type.c type.h \
	func.c func.h \
	main.c

ETAGS_ARGS = curl.scm

EXTRA_DIST = \
	curl.scm.in

BUILT_SOURCES = curl.scm
CLEANFILES = curl.scm

curl.scm: curl.scm.in
	if test "x$(RNRS_BYTEVECTORS)" = "xno" ; then \
	  cat curl.scm.in | sed -e 's|XXlibnameXX|libguile-curl|' -e 's|;; RNRS_BYTEVECTORS_BEGIN|#!|' -e 's|;; RNRS_BYTEVECTORS_END|!#|' > $@ ; \
	else \
	  cat curl.scm.in | sed -e 's|XXlibnameXX|libguile-curl|' > $@ ; \
	fi
