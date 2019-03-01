# Common makefile rules
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

# By default, build all targets
all: $(LIB_FILES) $(TARGET_FILES)

.PHONY : clean all header mwl

# The command to generate the parser...
$(PARSE_CFILE): %.c: %.y
	$(PARSE_GEN) $(PARSE_FLAGS) $<

$(PARSE_HFILE): %.h: %.y
	$(PARSE_GEN) $(PARSE_FLAGS) $<

# The command to generate the scanner...
$(SCAN_CFILE): %.c: %.l $(PARSE_CFILE)
	$(SCAN_GEN) $(SFLAGS) $<

# make the dependencies
# Make knows that a .cpp relies on the header that yacc generates, so it runs
# yacc before generating the dependency lists and including them.
#
# This is essentially the example included in the Gnu Make documentation (see
# Generating Prerequisites Automatically) except that in this case the .o's are
# not in the same directory as the .cpp's, hence the substitution in the sed
# command.
#
# The leading @ tells Make to silently execute the command
# set -e tells the shell (Bash specific?) to exit at the first error
# The $$$$ translate the the shell variable $$, which translates to the
# current pid.  For this reason, all of the trailing \'s are required to
# ensure that all the commands execute under the same shell invocation.
#
# Finally, the "-include" command includes the files that were just made, but
# not if the goal is to clean the directories.

$(DEP_C_FILES): $(OBJ_DIR)/%.d: %.c
	@set -e; rm -f $@; \
	$(CCc) -MM $(COMPILE_FLAGS) $(C_FLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(DEP_CPP_FILES): $(OBJ_DIR)/%.d: %.cpp
	@set -e; rm -f $@; \
	$(CC) -MM $(COMPILE_FLAGS) $(CPP_FLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(DEP_OBJC_FILES): $(OBJ_DIR)/%.d: %.m
	@set -e; rm -f $@; \
	$(CC) -MM $(COMPILE_FLAGS) $(OBJC_FLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(@:%.d=%.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),mwl)
-include $(DEP_FILES)
endif
endif

# The compile commands...
$(C_CMN_OBJ_FILES) $(C_TGT_OBJ_FILES): $(OBJ_DIR)/%.o: %.c
	$(CCc) $(COMPILE_FLAGS) $(C_FLAGS) $< -o $@
$(CPP_CMN_OBJ_FILES) $(CPP_TGT_OBJ_FILES): $(OBJ_DIR)/%.o: %.cpp
	$(CC) $(COMPILE_FLAGS) $(CPP_FLAGS) $< -o $@
$(OBJC_CMN_OBJ_FILES) $(OBJC_TGT_OBJ_FILES): $(OBJ_DIR)/%.o: %.m
	$(CC) $(COMPILE_FLAGS) $(OBJC_FLAGS) $< -o $@

# Make the libraries...
$(LIB_FILES): $(CMN_OBJ_FILES)
	ar -r $@ $(CMN_OBJ_FILES)
	ranlib $@

$(LEX_YACC_LIB_FILE): $(YACC_ARCHIVE) $(LEX_ARCHIVE)
	ar x $(YACC_ARCHIVE) yyerror.o
	ar x $(LEX_ARCHIVE) libyywrap.o
	ld -shared -o $@ yyerror.o libyywrap.o
	rm -f yyerror.o libyywrap.o

#libstdc++.so: /usr/lib/libstdc++.so.5.0.3
#	ln -s $< $@

# The link command...
$(TARGET_FILES): $(BIN_DIR)/%: $(OBJ_DIR)/%.o $(LIB_FILES) $(LINK_LIB_FILES)
	$(LD) $(LDFLAGS) $< $(LIBS:%=-l%) -o $@

clean:
	rm -f $(TARGET_FILES) $(OBJ_FILES) $(DEP_FILES) \
	$(PARSE_CFILE) $(PARSE_HFILE) $(SCAN_CFILE) $(LIB_FILES) \
	$(OTHER_MISC_FILES)
