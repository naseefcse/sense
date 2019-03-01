#
# Makefile for SENSE status file analyzer
#
# Copyright 2006 - Mark Lisee
#
# This file is free software; you can redistribute it and/or modify
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

include ../../misc/basicDefinitions.mk
include ../misc/projectDefinitions.mk

# Define the targets
C_TARGETS = 
CPP_TARGETS = findNeighbors displayNodes sortNodes countNeighbors connectivity
OBJC_TARGETS = 

# Define the files common to all targets
C_CMN_OBJS =
CPP_CMN_OBJS = parseRtns usage file commonRtns
OBJC_CMN_OBJS =
SCAN_FILE = stateScanner.l
PARSE_FILE = stateParser.y

# The file file.cpp (and file.h) are actually contained in the parseConfig tree
vpath file.cpp ../../libraries/parseConfig/src

# cause lex to generate debugging code
#SFLAGS += -d

# cause yacc to generate trace code
# also need to set yydebug=1 before calling parse()
#PARSE_FLAGS += -t

INC_PATHS = ../inc ../../libraries/parseConfig/inc
LIB_PATHS = ../lib

# Uncomment these lines as required for the directory
MAKE_PARSER_LIB    := YES
USE_PARSER_LIB     := YES

# optimization flags
#OPTIMIZATION = -O0 -ggdb
OPTIMIZATION = -O3 -march=pentium4 -mfpmath=sse

# flags for all source files
COMPILE_FLAGS += -Wall
# flags for specific languages
#C_FLAGS +=
#CPP_FLAGS +=
#OBJC_FLAGS +=

include ../../misc/basicRules.mk
