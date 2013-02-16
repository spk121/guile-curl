## Process this file with automake to produce Makefile.in
TESTS_ENVIRONMENT = LIBGUILE_CURL_MAJOR=$(LIBGUILE_CURL_MAJOR)
TESTS_ENVIRONMENT += ../libtool -dlopen ../src/libguile-curl.la \
	--mode=execute \
	guile -L $(top_srcdir)/src -L . -s

TESTS = setopt.scm \
	stress.scm

EXTRA_DIST = $(TESTS) \
	lib.scm \
	do_gdb
