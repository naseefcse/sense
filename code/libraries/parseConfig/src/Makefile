#
# Makefile for a directory
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

include ../../../misc/basicDefinitions.mk
include ../../misc/projectDefinitions.mk

# Define the targets
C_TARGETS = 
CPP_TARGETS = 
OBJC_TARGETS = 

# Define the files common to all targets
C_CMN_OBJS =
CPP_CMN_OBJS = CIParseRtns ConfigInfo ConfigInfoWrapper file NodeInfo
OBJC_CMN_OBJS =
SCAN_FILE = CIScanner.l
PARSE_FILE = CIParser.y

#SFLAGS += -d

INC_PATHS = ../inc ../../visualizer/inc ../../../common
LIB_PATHS = ../lib

# Uncomment these lines as required for the directory
MAKE_PARSER_LIB    := YES
#USE_PARSER_LIB     := YES

# flags for all source files
#COMPILE_FLAGS +=
# flags for specific languages
#C_FLAGS +=
#CPP_FLAGS +=
#OBJC_FLAGS +=

include ../../../misc/basicRules.mk
