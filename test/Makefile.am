## Process this file with automake to produce Makefile.in
all-local:
	-ln -s ../src $(PACKAGE)


TESTS_ENVIRONMENT = LIBGUILE_CURL_MAJOR=$(LIBGUILE_CURL_MAJOR)
TESTS_ENVIRONMENT += ../libtool -dlopen ../src/libguile-curl.la \
	--mode=execute \
	guile -L $(top_srcdir)/src -L $(top_srcdir) -s

TESTS = t001_test.test

CLEANFILES =  *.out

EXTRA_DIST = $(TESTS) \
	lib.scm \
	lib2.scm \
	do_gdb
