## Process this file with automake to produce Makefile.in

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

CLEANFILES = guile-curl.dvi
