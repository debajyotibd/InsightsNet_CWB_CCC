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


##  This is the Makefile for the CWB Corpus Library. 



## Read configuration settings and standard definitions
TOP = $(shell pwd)/..
include $(TOP)/config.mk

## The following targets are available:
#
#  all          create archive libcl.a
#  clean        remove object files and libcl.a
#  realclean    also delete automatically generated parsers and dependencies
#  depend       update dependencies
#  grammar      compile flex/bison parser
#  install      install libcl.a and CL headers
#  uninstall    uninstall libcl.a and headers from chosen locations (currently not supported)
#  release      install to binary release dir
#  size         print size of source code (line counts)
#

.PHONY: all clean realclean depend install uninstall size

## Use bison/flex to compile parser for registry entries
PARSEH  = registry.tab.h
PARSES  = registry.tab.c lex.creg.c
PARSEG  = registry.l registry.y
PARSEO  = registry.tab.o lex.creg.o

## Compile and link-in windows-mmap functions only on Windows
ifdef WINDOWS
  MMAPH = windows-mmap.h
  MMAPS = windows-mmap.c
  MMAPO = windows-mmap.o
else
  MMAPH =
  MMAPS =
  MMAPO =
endif

## cl.h header file will be installed in subdirectory cwb/
REALINCINSTDIR = $(INCINSTDIR)/cwb

## ----------------------------------------------------------------------
## CORPUS LIBRARY  headers / sources / objects 

HDRS = globals.h cwb-globals.h ui-helpers.c \
       bitfields.h storage.h fileutils.h \
       regtab.h special-chars.h \
       corpus.h attributes.h makecomps.h \
       $(PARSEH) \
       bitio.h \
       endian.h \
       compression.h \
       $(MMAPH)

SRCS = globals.c macros.c ui-helpers.c \
       list.c lexhash.c ngram-hash.c \
       bitfields.c storage.c fileutils.c \
       regtab.c special-chars.c regopt.c \
       corpus.c attributes.c makecomps.c \
       $(PARSES) \
       cdaccess.c \
       bitio.c \
       endian.c \
       compression.c \
       $(MMAPS)
 
OBJS = globals.o macros.o ui-helpers.o \
       list.o lexhash.o ngram-hash.o \
       bitfields.o storage.o fileutils.o \
       regtab.o special-chars.o regopt.o \
       corpus.o attributes.o makecomps.o \
       $(PARSEO) \
       cdaccess.o \
       bitio.o \
       endian.o \
       compression.o \
       $(MMAPO)

## Extra objects (will be compiled, but not included in libcl.a) 
##   - dl_stub.o is a dummy replacement for libdl.a, which is missing in Solaris
EXTRA_OBJS = dl_stub.o

# ----------------------------------------------------------------------

all: libcl.a $(LIBCL_SHAREDLIB) $(EXTRA_OBJS)

registry.tab.c registry.tab.h: registry.y corpus.h attributes.h globals.h cl.h
	@$(ECHO) "    .... compiling registry parser from Bison grammar"
	$(YACC) -p creg -v registry.y
	# NB: see comments in registry.y on use of "-p creg".

lex.creg.c: registry.l registry.y registry.tab.h corpus.h attributes.h globals.h cl.h
	@$(ECHO) "    .... compiling registry lexer from Flex grammar"
	$(LEX) registry.l

libcl.a: $(OBJS)
	@$(ECHO) "    .... generating static library file for CL (libcl.a)"
	$(RM) $@
	$(AR) $@ $^
	$(RANLIB) $@

libcl.so: $(OBJS)
	@$(ECHO) "    .... generating shared library file for CL (libcl.so)"
	$(CC) -shared -Wl,-soname,libcl.so -o libcl.so $(OBJS) $(LIBCL_EXTRA_LIBS) $(LDFLAGS_ALL)

libcl.dylib: $(OBJS)
	@$(ECHO) "    .... generating shared library file for CL (libcl.dylib)"
	$(CC) -dynamiclib -install_name '$(FINALLIBINSTDIR)/libcl.dylib' -compatibility_version 3.4 -current_version $(VERSION) -o libcl.dylib $(OBJS) $(LIBCL_EXTRA_LIBS) $(LDFLAGS_ALL)

libcl.dll libcl.dll.a libcl.lib &: $(OBJS)
	@$(ECHO) "    .... generating shared library file for CL (libcl.dll, libcl.lib)"
ifdef WINDOWS_RELEASE_EXT
	$(CC) -shared -o libcl.dll $(SRCS) -Wl,--out-implib,libcl.lib $(CFLAGS_ALL) $(LIBCL_EXTRA_LIBS) $(LDFLAGS_ALL)
else 
  ifdef WINDOWS
	## building shared lib for internal installation into MSYS2. note that the ".lib" above is now ".dll.a". This is still UNTESTED.
	$(CC) -shared -o libcl.dll $(SRCS) -Wl,--out-implib,libcl.dll.a $(CFLAGS_ALL) $(LDFLAGS_ALL)
  endif
endif

ifndef WINDOWS_RELEASE_EXT
install: libcl.a $(LIBCL_SHAREDLIB)
	@$(ECHO) "    .... installing cwb/cl.h header file"
	if [ ! -d "$(REALINCINSTDIR)" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(REALINCINSTDIR)" ; fi;
	$(INSTALL) $(INST_FLAGS_DATA) cl.h "$(REALINCINSTDIR)"
	
	@$(ECHO) "    .... installing the static CL library file"
	if [ ! -d "$(LIBINSTDIR)" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(LIBINSTDIR)" ; fi;
	$(INSTALL) $(INST_FLAGS_DATA) libcl.a "$(LIBINSTDIR)"
	
	@$(ECHO) "    .... installing the dynamic CL library file"
	if [ ! -d "$(LIBINSTDIR)" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(LIBINSTDIR)" ; fi;
	$(INSTALL) $(INST_FLAGS_DATA) $(LIBCL_SHAREDLIB) "$(LIBINSTDIR)"
else
install:
	@$(ECHO) "ERROR: install operation is not supported for Windows binaries built for release outside MSYS2!"
endif

release: $(LIBCL_SHAREDLIB)
	## don't include libcl.a in binary release because it's useless without the prerequisite libraries
	## and libcl.dll plus libcl.lib are statically compiled, and can be linked against.
	@$(ECHO) "    .... add to release: cwb/cl.h header file"
	if [ ! -d "$(RELEASE_DIR)/include/cwb" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(RELEASE_DIR)/include/cwb"; fi;
	$(INSTALL) $(INST_FLAGS_DATA) cl.h "$(RELEASE_DIR)/include/cwb"
	@$(ECHO) "    .... add to release: libcl shared library files"
	if [ ! -d "$(RELEASE_DIR)/lib" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(RELEASE_DIR)/lib"; fi;
	for i in $(LIBCL_SHAREDLIB) ; do $(INSTALL) $(INST_FLAGS_DATA) "$$i" "$(RELEASE_DIR)/lib" ; done
	
uninstall:
	@$(ECHO) "ERROR: uninstall operation is not supported!"

depend:
	@$(ECHO) "    .... generating the CL's make dependencies"
	-$(RM) depend.mk
	$(MAKE) depend.mk

## be sure to create bison/flex C code before making dependencies
depend.mk: $(PARSEH) $(PARSES) $(SRCS)
	-$(RM) depend.mk
	$(DEPEND) $(DEPEND_CFLAGS_ALL) $(SRCS) > depend.mk

## compile bison/flex parser for source release (tarball), but not depend.mk
grammar: $(PARSEH) $(PARSES) 
	-$(RM) depend.mk

clean:
	@$(ECHO) "    .... cleanup compiled CL library code"
	-$(RM) *.o libcl.a registry.output *~ $(LIBCL_SHAREDLIB)

realclean: clean
	@$(ECHO) "    .... cleanup CL registry parser's automatic files"
	-$(RM) registry.tab.c registry.tab.c lex.creg.c registry.tab.h
	-$(RM) depend.mk

size: $(SRCS) $(HDRS)
	@$(ECHO) "    .... count up lines of code in CL"
	@$(WC) $(SRCS) $(HDRS) 


# -------- dependencies --------
include depend.mk

