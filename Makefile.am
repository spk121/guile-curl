## Process this file with automake to produce Makefile.in

SUBDIRS = \
	src \
	m4 \
	doc \
	test \
	build-aux

ACLOCAL_AMFLAGS = -I m4 --install

# Gnulib
distclean-local: clean-GNUmakefile
clean-GNUmakefile:
	test '$(srcdir)' = . || rm -f $(top_builddir)/GNUmakefile

EXTRA_DIST = \
	AUTHORS \
	INSTALL \
	NEWS \
	README \
	COPYING \
	ChangeLog \
	m4/gnulib-cache.m4 \
	build-aux/texinfo.tex \
	cfg.mk
