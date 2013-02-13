## Process this file with automake to produce Makefile.in

### MAIN TARGETS ###

guileextension_LTLIBRARIES = \
	libguile-curl.la

dist_guilesite_DATA = \
	curl.scm

####################

libguile_curl_la_CPPFLAGS = $(GUILE_CFLAGS) -pthread

libguile_curl_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info $(LIBGUILE_CURL_INTERFACE)

libguile_curl_la_SOURCES = \
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
	cat $< | \
	sed 's|XXlibnameXX|libguile-curl|' > $@
