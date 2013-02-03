## Process this file with automake to produce Makefile.in

COMPILE_FLAGS = -Wall -Wextra

lib_LTLIBRARIES = \
	libguile-curl.la

libguile_curl_la_CPPFLAGS = $(COMPILE_FLAGS) $(GUILE_CFLAGS)

libguile_curl_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info @LIBGUILE_CURL_INTERFACE@

libguile_curl_la_SOURCES = \
	const.c const.h \
	type.c type.h \
	func.c func.h \
	main.c

guilemodule_DATA = curl.scm
ETAGS_ARGS = curl.scm

EXTRA_DIST = \
	curl.scm.in

BUILT_SOURCES = curl.scm
CLEANFILES = curl.scm

curl.scm: curl.scm.in
	cat $< | \
	sed 's|XXlibnameXX|libguile-curl|' > $@
