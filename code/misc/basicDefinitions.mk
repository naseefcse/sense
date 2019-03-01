# Common makefile definitions
#
# Copyright 2005, 2007 - Mark Lisee
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

BIN_DIR := ../bin
OBJ_DIR := ../obj
LIB_DIR := ../lib

# Declare the scanner generator and supply the flags and output file
#
SCAN_GEN = lex
SCAN_CFILE = $(SCAN_FILE:%.l=%.c)
SFLAGS = -o$(SCAN_CFILE)
LEX_ARCHIVE = /usr/lib/libl.a

# Declare the parser generator and supply the flags and output file
#
PARSE_GEN = yacc
PARSE_CFILE = $(PARSE_FILE:%.y=%.c)
PARSE_HFILE = $(PARSE_FILE:%.y=%.h)
PARSE_FLAGS = -d -o $(PARSE_CFILE)
YACC_ARCHIVE = /usr/lib/liby.a

#PROFILE_FLAGS = -pg -g -ftest-coverage

# Declare the compiler and supply compile the flags
#
CC = g++
CCc = gcc
INC_FLAGS=$(INC_PATHS:%=-I%)

# Flags used on all compiles
COMPILE_FLAGS = -Wall -c $(INC_FLAGS) $(OPTIMIZATION) $(PROFILE_FLAGS) \
	$(CCFLAGS) $(DEBUG_FLAGS)

# Flags used only for C files
C_FLAGS = 

# Flags used only for C++ files
CPP_FLAGS =

# Flags used only for Objective C files
OBJC_FLAGS =

# Declare the linker, supply the link flags and the libraries.
#LD = /home/mlisee/temp/usr/local/bin/gcc
LD = gcc
# GSL is linked shared
LDSHARED = #-Wl,--rpath -Wl,/usr/local/lib
LIB_FLAGS = $(LIB_PATHS:%=-L%)
LDFLAGS = $(LIB_FLAGS) $(OPTIMIZATION) $(LDSHARED) $(PROFILE_FLAGS)

# convert the object names into object files
C_CMN_OBJ_FILES = $(PARSE_FILE:%.y=$(OBJ_DIR)/%.o) $(SCAN_FILE:%.l=$(OBJ_DIR)/%.o)
C_CMN_OBJ_FILES += $(C_CMN_OBJS:%=$(OBJ_DIR)/%.o)
CPP_CMN_OBJ_FILES = $(CPP_CMN_OBJS:%=$(OBJ_DIR)/%.o)
OBJC_CMN_OBJ_FILES = $(OBJC_CMN_OBJS:%=$(OBJ_DIR)/%.o)
CMN_OBJ_FILES = $(C_CMN_OBJ_FILES) $(OBJC_CMN_OBJ_FILES) $(CPP_CMN_OBJ_FILES)

# convert target names into source file names and object file names
C_TGT_OBJ_FILES = $(C_TARGETS:%=$(OBJ_DIR)/%.o)
TARGET_FILES = $(C_TARGETS:%=$(BIN_DIR)/%)
CPP_TGT_OBJ_FILES = $(CPP_TARGETS:%=$(OBJ_DIR)/%.o)
TARGET_FILES += $(CPP_TARGETS:%=$(BIN_DIR)/%)
OBJC_TGT_OBJ_FILES = $(OBJC_TARGETS:%=$(OBJ_DIR)/%.o)
TARGET_FILES += $(OBJC_TARGETS:%=$(BIN_DIR)/%)
TGT_OBJ_FILES = $(C_TGT_OBJ_FILES) $(CPP_TGT_OBJ_FILES) $(OBJC_TGT_OBJ_FILES)

# lists of object files by source language
C_OBJ_FILES = $(C_TGT_OBJ_FILES) $(C_CMN_OBJ_FILES)
CPP_OBJ_FILES = $(CPP_TGT_OBJ_FILES) $(CPP_CMN_OBJ_FILES)
OBJC_OBJ_FILES = $(OBJC_TGT_OBJ_FILES) $(OBJC_CMN_OBJ_FILES)

# list of all object files
OBJ_FILES = $(CMN_OBJ_FILES) $(TGT_OBJ_FILES)

DEP_C_FILES = $(C_CMN_OBJ_FILES:%.o=%.d) $(C_TGT_OBJ_FILES:%.o=%.d)
DEP_CPP_FILES = $(CPP_CMN_OBJ_FILES:%.o=%.d) $(CPP_TGT_OBJ_FILES:%.o=%.d)
DEP_OBJC_FILES = $(OBJC_CMN_OBJ_FILES:%.o=%.d) $(OBJC_TGT_OBJ_FILES:%.o=%.d)
DEP_FILES = $(DEP_CPP_FILES) $(DEP_C_FILES) $(DEP_OBJC_FILES)

# Include the correct libraries in the link command
LIBS = $(if $(strip $(CPP_OBJ_FILES)), stdc++)
LIBS += $(if $(strip $(OBJC_OBJ_FILES)), objc pthread)
LIBS += $(DEBUG_LIBS)
