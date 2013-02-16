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


SUBDIRS = \
	src \
	m4 \
	doc \
	lib \
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
	TODO \
	ChangeLog \
	m4/gnulib-cache.m4 \
	build-aux/texinfo.tex \
	GNUmakefile \
	maint.mk \
	cfg.mk
