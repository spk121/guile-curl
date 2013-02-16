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

