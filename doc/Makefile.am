## Process this file with automake to produce Makefile.in

# Copyright (C) 2011, 2013, 2014 Michael L. Gran

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

# Cygwin's texi2dvi can try to call etex, resulting in this error:
#   "texinfo.tex appears to be broken, quitting"
# The problem lies in Cygwin's etex.fmt file
# Forcing TEX to tex works around this problem
# This should not harm other platforms

# if ETEX_FIX
# TEX=tex
# TEXI2DVI = TEX=${TEX} texi2dvi
# endif

info_TEXINFOS = guile-curl.texi
guile_curl_TEXINFOS = ex_http_header_charset.scm

CLEANFILES = guile-curl.dvi
