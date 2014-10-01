# Configuration for maintainer-makefile
#
# Copyright (c) 2014 Michael L. Gran
#
# This file is part of guile-curl
#
# guile-aspell is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# guile-aspell is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with guile-aspell; see the file COPYING.  If not, write to the
# Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
# MA 02111-1301, USA.

# Set format of NEWS
old_NEWS_hash :=

local-checks-to-skip = \
	sc_bindtextdomain \
	sc_error_message_uppercase \
	sc_immutable_NEWS \
	sc_program_name \
	sc_unmarked_diagnostics
