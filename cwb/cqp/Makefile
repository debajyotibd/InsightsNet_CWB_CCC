##   -*-Makefile-*-
## 
##  IMS Open Corpus Workbench (CWB)
##  Copyright (C) 1993-2006 by IMS, University of Stuttgart
##  Copyright (C) 2007-     by the respective contributers (see file AUTHORS)
## 
##  This program is free software; you can redistribute it and/or modify it
##  under the terms of the GNU General Public License as published by the
##  Free Software Foundation; either version 2, or (at your option) any later
##  version.
## 
##  This program is distributed in the hope that it will be useful, but
##  WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
##  Public License for more details (in the file "COPYING", or available via
##  WWW at http://www.gnu.org/copyleft/gpl.html).


##  This is the Makefile for the CWB Corpus Query Processor programs. 

## The following targets are available:
#
#  all          create binaries cqp, cqpcl, and cqpserver
#  clean        remove object files and binaries
#  realclean    also deleted automatically generated parsers and dependencies
#  depend       update dependencies
#  grammar      compile flex/bison parser
#  install      install binaries
#  uninstall    uninstall binaries from chosen location (currently not supported)
#  release      install to binary release dir
#  size         print size of source code (line counts)
#


## Read configuration settings and standard definitions
TOP = $(shell pwd)/..
include $(TOP)/config.mk

## cqi.h header file will be installed in subdirectory cwb/
REALINCINSTDIR = $(INCINSTDIR)/cwb

## This is an extra flag for this folder. It affects the CQP parser; 
## normally it is not defined, but if it is, the synchronize() function 
## becomes a no-op.
ifdef CQP_COMPILE_WITHOUT_SYNCHRONIZE
  CFLAGS_ALL := -DCQP_COMPILE_WITHOUT_SYNCHRONIZE $(CFLAGS_ALL) 
endif

.PHONY: all clean realclean depend install uninstall size

## Use bison/flex to compile parser for registry entries
PARSEH = parser.tab.h
PARSES = parser.tab.c lex.yy.c
PARSEG = parser.l parser.y
PARSEO = parser.tab.o lex.yy.o

## ----------------------------------------------------------------------
## CQP & CQPserver  headers / sources / objects / binaries

SRCS =  llquery.c cqp.c cqpcl.c symtab.c eval.c tree.c options.c corpmanag.c \
	regex2dfa.c output.c ranges.c builtins.c groups.c targets.c \
	matchlist.c \
	concordance.c \
	parse_actions.c attlist.c context_descriptor.c \
	print-modes.c ascii-print.c sgml-print.c html-print.c latex-print.c \
	variables.c print_align.c \
	macro.c \
	../CQi/server.c ../CQi/auth.c ../CQi/log.c \
	$(PARSES)

OBJS =  cqp.o symtab.o eval.o tree.o options.o \
	corpmanag.o regex2dfa.o output.o ranges.o builtins.o \
	groups.o targets.o matchlist.o \
	concordance.o \
	parse_actions.o attlist.o context_descriptor.o \
	print-modes.o ascii-print.o sgml-print.o html-print.o latex-print.o \
	variables.o print_align.o \
	macro.o \
	$(PARSEO)

## CQPserver needs additional objects. These are not in the general list, so we can exclude them when linking CQP
## (otherwise CQP needs to be linked against network libraries and we've just had a compatibility problem with that). 
## However, since the parser uses functions from <auth.h> (in the 'user' and 'host' commands), 
## we must provide dummy implementations of those functions for CQP. 
CQI_OBJS = ../CQi/server.o ../CQi/auth.o ../CQi/log.o
CQP_OBJS = dummy_auth.o

HDRS =  cqp.h options.h symtab.h tree.h eval.h corpmanag.h \
	regex2dfa.h output.h \
	ranges.h builtins.h treemacros.h \
	groups.h targets.h matchlist.h \
	concordance.h \
	parse_actions.h attlist.h context_descriptor.h \
	print-modes.h ascii-print.h sgml-print.h html-print.h latex-print.h \
	variables.h print_align.h \
	macro.h \
	../CQi/server.h ../CQi/auth.h ../CQi/cqi.h \
	$(PARSEH)

PROGRAMS = cqp$(EXEC_SUFFIX) cqpcl$(EXEC_SUFFIX) cqpserver$(EXEC_SUFFIX)

## ----------------------------------------------------------------------

all: $(PROGRAMS)

parser.tab.c parser.tab.h: parser.y
	@$(ECHO) "    .... compiling CQP parser from Bison grammar"
	$(RM) parser.tab.* parser.output
	$(YACC) $<

lex.yy.c: parser.l parser.y
	@$(ECHO) "    .... compiling CQP lexer from Flex grammar"
	$(RM) lex.yy.*
	$(LEX) $<

cqp$(EXEC_SUFFIX): $(OBJS) $(CQP_OBJS) llquery.o $(LIBCL_PATH)
	@$(ECHO) "    .... linking executable" $@
	$(RM) $@
	$(CC) $(CFLAGS_ALL) -o $@ llquery.o $(OBJS) $(CQP_OBJS) $(CL_LIBS) $(LIB_REGEX) $(LDFLAGS_ALL) $(LDFLAGS_CQP)

cqpcl$(EXEC_SUFFIX): $(OBJS) $(CQP_OBJS) cqpcl.o $(LIBCL_PATH)
	@$(ECHO) "    .... linking executable" $@ 
	$(RM) $@
	$(CC) $(CFLAGS_ALL) -o $@ cqpcl.o $(OBJS) $(CQP_OBJS) $(CL_LIBS) $(LIB_REGEX) $(LDFLAGS_ALL) $(LDFLAGS_CQP)

cqpserver$(EXEC_SUFFIX): $(OBJS) $(CQI_OBJS) ../CQi/cqpserver.o $(LIBCL_PATH)
	@$(ECHO) "    .... linking executable" $@ 
	$(RM) $@
	$(CC) $(CFLAGS_ALL) -o $@ ../CQi/cqpserver.o $(OBJS) $(CQI_OBJS) $(CL_LIBS) $(LIB_REGEX) $(LDFLAGS_ALL) $(LDFLAGS_CQP) $(NETWORK_LIBS)

depend:
	@$(ECHO) "    .... generating CQP's make dependencies"
	-$(RM) depend.mk
	$(MAKE) depend.mk

## be sure to create bison/flex C code before making dependencies
depend.mk: $(PARSEH) $(PARSES) $(SRCS)
	-$(RM) depend.mk
	$(DEPEND) $(DEPEND_CFLAGS_ALL) $(SRCS) > depend.mk

# compile bison/flex parser for source release (tarball), but delete depend.mk so it does not get included.
grammar: $(PARSEH) $(PARSES) 
	-$(RM) depend.mk

install: $(PROGRAMS)
	@$(ECHO) "    .... installing the CQP programs"
	if [ ! -d $(BININSTDIR) ]; then $(INSTALL) $(INST_FLAGS_DIR) $(BININSTDIR); fi;
	for i in $(PROGRAMS) ; do $(INSTALL) $(INST_FLAGS_BIN) $$i $(BININSTDIR) ; done
	@$(ECHO) "    .... installing the CQi/cqi.h header file"
	if [ ! -d "$(REALINCINSTDIR)" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(REALINCINSTDIR)" ; fi;
	$(INSTALL) $(INST_FLAGS_DATA) ../CQi/cqi.h $(REALINCINSTDIR)

release: $(PROGRAMS)
	@$(ECHO) "    .... adding to release: the CQP programs"
	if [ ! -d "$(RELEASE_DIR)/bin" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(RELEASE_DIR)/bin"; fi;
	for i in $(PROGRAMS); do $(INSTALL) $(INST_FLAGS_BIN) $$i "$(RELEASE_DIR)/bin"; done
	@$(ECHO) "    .... adding to release: the CQi/cqi.h header file"
	if [ ! -d "$(RELEASE_DIR)/include/cwb" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(RELEASE_DIR)/include/cwb"; fi;
	$(INSTALL) $(INST_FLAGS_DATA) ../CQi/cqi.h "$(RELEASE_DIR)/include/cwb"

uninstall:
	@$(ECHO) "ERROR: uninstall operation is not supported!"

clean:
	@$(ECHO) "    .... cleaning up CQP programs"
	-$(RM) $(PROGRAMS) $(NO_INSTALL) *.o ../CQi/*.o parser.output *~ ../CQi/*~
	# TODO what is "parser.output" ? PARSES, PARSEH, PARSEO I think ....

realclean: clean
	@$(ECHO) "    .... cleaning up CQP parser code"
	-$(RM) lex.yy.c parser.tab.c parser.tab.h 
	-$(RM) depend.mk

## create parser C source, if necessary
size: $(SRCS) $(HDRS)
	@$(ECHO) "    .... counting up lines of code in CQP"
	@$(WC) $(SRCS) $(HDRS)


# -------- dependencies --------
include depend.mk



