## Process this file with automake to produce Makefile.in

### MAIN TARGETS ###

guileextension_LTLIBRARIES = \
	libguile-curl.la

dist_guilesite_DATA = \
	curl.scm

####################

libguile_curl_la_CPPFLAGS = \
	$(CURL_CFLAGS) \
	$(GUILE_CFLAGS) \
	$(CFLAG_VISIBILITY)

libguile_curl_la_LDFLAGS = \
	-no-undefined \
	-export-dynamic \
	-version-info $(LIBGUILE_CURL_INTERFACE) \
	$(CURL_LDFLAGS) \
	$(GUILE_LDFLAGS)

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
	if test "x$(RNRS_BYTEVECTORS)" = "xno" ; then \
	  cat curl.scm.in | sed -e 's|XXlibnameXX|libguile-curl|' -e 's|;; RNRS_BYTEVECTORS_BEGIN|#!|' -e 's|;; RNRS_BYTEVECTORS_END|!#|' > $@ ; \
	else \
	  cat curl.scm.in | sed -e 's|XXlibnameXX|libguile-curl|' > $@ ; \
	fi
