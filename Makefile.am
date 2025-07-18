## Process this file with automake to produce Makefile.in

# Copyright (C) 2011, 2013, 2021 Michael L. Gran

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

ACLOCAL_AMFLAGS = -I m4 --install

#################
#  Directories  #
#################

exampledir = $(datadir)/doc/guile-curl
guilesitedir = $(datadir)/guile/site/$(GUILE_EFFECTIVE_VERSION)
guileextensiondir = $(libdir)/guile/$(GUILE_EFFECTIVE_VERSION)/extensions

#################
#   Top Level   #
#################

#################
# Documentation #
#################

info_TEXINFOS = doc/guile-curl.texi
dist_example_DATA = doc/ex_http_simple_get.scm \
	doc/ex_http_simple_get_utf8.scm \
	doc/ex_http_web_response.scm

# In make distcheck, avoid building a doc format that requires TeX.
AM_DISTCHECK_DVI_TARGET = html

###################
#  libguile-curl  #
###################

guileextension_LTLIBRARIES = src/libguile-curl.la

src_libguile_curl_la_c_sources = \
	src/const.c \
	src/type.c \
	src/func.c \
	src/main.c

src_libguile_curl_la_internal_headers = \
	src/visibility.h \
	src/const.h \
	src/type.h \
	src/func.h

src_libguile_curl_la_SOURCES = \
  $(src_libguile_curl_la_internal_headers) \
  $(src_libguile_curl_la_c_sources)

AM_CFLAGS = \
	$(CURL_CFLAGS) \
	$(GUILE_CFLAGS)

ETAGS_ARGS = src/curl.scm

src_libguile_curl_la_LDFLAGS = \
 -version-info $(LIBGUILE_CURL_INTERFACE)

src_libguile_curl_la_LIBADD = $(CURL_LIBS) $(GUILE_LIBS)

if BUILDING_DLL
AM_CFLAGS += -DCL_DLL -DCL_DLL_EXPORTS
src_libguile_curl_la_LDFLAGS += -no-undefined
# For MinGW, force the dll to be installed in $(guileextensiondir)
# instead of $(guileextensiondir)/../bin
src_libguile_curl_la_LDFLAGS += -bindir $(DESTDIR)$(guileextensiondir)
endif
if !DLL_VERSION_INFO
src_libguile_curl_la_LDFLAGS += -avoid-version
endif

#################
# Guile Modules #
#################

dist_guilesite_DATA = \
 module/curl.scm

if GUILD
$(top_builddir)/module/curl.go: $(top_builddir)/src/libguile-curl.la

guileobject_DATA = \
  $(dist_guilesite_DATA:%.scm=%.go)

GUILEC_FLAGS = \
 -O2 \
 -Warity-mismatch \
 -Wformat \
 -Wmacro-use-before-definition \
 -Wunbound-variable \
 --load-path=$(abs_srcdir)/module \
 --load-path=$(abs_builddir)/module

.scm.go:
	GUILE_AUTO_COMPILE=0 \
	LTDL_LIBRARY_PATH=$(abs_builddir)/src/.libs \
	$(GUILD) compile --target="$(host)" $(GUILEC_FLAGS) \
	-o "$@" "$<"
endif

#################
#     Tests     #
#################

TEST_EXTENSIONS = .scm

TESTS_LTDLOPEN = -dlopen ${abs_top_builddir}/src/libguile-curl.la
TESTS = \
  test/bytevectors.scm \
  test/setopt.scm \
  test/stress.scm \
  test/getinfo.scm

SCM_LOG_DRIVER = \
  $(top_builddir)/libtool $(TESTS_LTDLOPEN) --mode=execute \
  $(GUILE) --no-auto-compile \
   -L $(abs_srcdir)/module \
   -L $(abs_builddir)/module \
   $(top_srcdir)/build-aux/test-driver.scm

LOG_DRIVER = \
  $(top_srcdir)/build-aux/test-driver

check_PROGRAMS = check_type

check_type_SOURCES = \
	test/check_type.c \
	$(src_libguile_curl_la_SOURCES)
check_type_CFLAGS = $(CURL_CFLAGS) $(GUILE_CFLAGS)
check_type_LDFLAGS = $(CURL_LIBS) $(GUILE_LIBS)

TESTS += check_type

test: check

#################
#   Variables   #
#################

CLEANFILES = \
  doc/guile-gi.dvi \
  examples/resources/test.gresource \
  $(guileobject_DATA) $(pkgguileobject_DATA) $(scriptguileobject_DATA) \
  $(BUILT_SOURCES) \
  $(check_DATA) \
  *.log

EXTRA_DIST = \
	AUTHORS \
	NEWS \
	README \
	COPYING \
	TODO \
	m4/guile.m4 \
	build-aux/texinfo.tex \
	build-aux/ar-lib build-aux/compile build-aux/missing \
	build-aux/test-driver build-aux/test-driver.scm \
	build-aux/config.rpath \
	$(TESTS) \
	test/profile.scm


SUFFIXES =
