#
# project wide makefile definitions
#
# Copyright 2006 - Mark Lisee
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# Project wide compile flags
include ../../../optimizations.mk
include ../../../definitions.mk


# A project that uses multiple libraries will list them here.
# Most likely, each directory's makefile will define USE_foo_LIB and/or
# MAKE_foo_LIB to enable linking with or creating the library
PARSER_LIB = configParser
VISUALIZER_LIB = visualizer

# Include the correct libraries in the link command
LIBS += $(if $(strip $(USE_PARSER_LIB)), $(PARSER_LIB))
LIBS += $(if $(strip $(USE_VISUALIZER_LIB)), $(VISUALIZER_LIB))
# This example includes the lex & yacc libraries
LIBS += $(if $(strip $(USE_PARSER_LIB)), $(PARSER_LIB) y l)

# convert the library names into library files
PARSER_LIB_FILES = $(PARSER_LIB:%=$(LIB_DIR)/lib%.a)
VISUALIZER_LIB_FILES = $(VISUALIZER_LIB:%=$(LIB_DIR)/lib%.a)

# define LIB_FILES only if MAKE_xxx_LIB is non-NULL
# This means that only the directory that creates a library will delete it.
LIB_FILES  = $(if $(strip $(MAKE_PARSER_LIB)), $(PARSER_LIB_FILES))
LIB_FILES += $(if $(strip $(MAKE_VISUALIZER_LIB)), $(VISUALIZER_LIB_FILES))

# Add the library files to the dependency list, but only if they aren't being
# made by this directory
LINK_LIB_FILES  = $(if $(strip $(USE_PARSER_LIB)), $(if $(strip $(MAKE_PARSER_LIB)), , $(PARSER_LIB:%=lib%.a)))
LINK_LIB_FILES += $(if $(strip $(USE_VISUALIZER_LIB)), $(if $(strip $(MAKE_VISUALIZER_LIB)), , $(VISUALIZER_LIB:%=lib%.a)))
