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


#
# ***** This file should NOT be edited! *****
#


#
# CWB version
#
VERSION = 3.5.1
# 3.5.x = long-term stable support version.
# 3.6.x = novel developments after 3.5; considered unstable.


#
# Check that required configuration variables are set
#

ifndef DEFAULT_REGISTRY
  $(error Configuration variable DEFAULT_REGISTRY is not set (default registry directory))
endif

ifndef PREFIX
  $(error Configuration variable PREFIX is not set (software installation tree))
endif

ifndef INST_PERM
  $(error Configuration variable INST_PERM is not set (access permissions for installed files))
endif

ifndef CC
  $(error Configuration variable CC is not set (C compiler to use))
endif

# CFLAGS and LDFLAGS may be empty if no special flags or libraries are needed,
# so their absence should not trigger an error. 

# ifndef CFLAGS
#   $(error Configuration variable CFLAGS is not set (C compiler options))
# endif

# ifndef LDFLAGS
#   $(error Configuration variable LDFLAGS is not set (linker options))
# endif

ifndef YACC
  $(error Configuration variable YACC is not set (yacc or bison parser))
endif

ifndef LEX
  $(error Configuration variable LEX is not set (lex or flex scanner))
endif

ifndef AR
  $(error Configuration variable AR is not set (for building archive from .o files))
endif

ifndef RANLIB
  $(error Configuration variable RANLIB is not set (make table of contents for .a files))
endif


#
# Variables that are normally set here but may have to be overridden in special situations
#

ifndef ETAGS
  ETAGS = $(error Cannot build TAGS file, no ETAGS program given in configuration)
endif

ifndef DEPEND
  DEPEND = $(error Cannot update dependencies, no DEPEND program call given in configuration)
endif

ifndef DEPEND_CFLAGS
  DEPEND_CFLAGS = $(CFLAGS)
endif

# many systems have a GNU-compatible install that may be faster than the included script
ifndef INSTALL
  INSTALL = $(TOP)/instutils/install.sh -c
endif

# must be set by platform config if `date` doesn't work (or if one wants to lie)
ifndef COMPILE_DATE
  COMPILE_DATE = "$(shell date)"
endif


#
# Command line executable names and always-needed flags: these should almost never need to be changed
#

ifndef CHMOD
  CHMOD = chmod
endif
ifndef CP
  CP = cp
endif
ifndef ECHO
  ECHO = echo
endif
ifndef RM
  RM = rm -f
endif
ifndef WC
  WC = wc -l
endif
ifndef TAR
  TAR = tar
endif
## ZIP is only needed by "make release" for Windows
ifndef ZIP
  ZIP = zip -r
endif
## SED is only needed by "make deb-release" for Linux
ifndef SED
  SED = sed
endif


#
# Command-line flags for (GNU-compatible) install program
#

INST_FLAGS = 
ifndef WINDOWS
  ifdef INST_USER
    INST_FLAGS += -o $(INST_USER)
  endif
  ifdef INST_GROUP
    INST_FLAGS += -g $(INST_GROUP)
  endif
endif
INST_PERM_DATA = $(subst 7,6,$(subst 5,4,$(subst 3,2,$(subst 1,0,$(INST_PERM)))))
INST_PERM_BIN  = $(subst 6,7,$(subst 4,5,$(subst 2,3,$(subst 0,0,$(INST_PERM_DATA)))))

INST_FLAGS_DATA = $(INSTFLAGS_FILE) -m $(INST_PERM_DATA) $(INST_FLAGS)
INST_FLAGS_BIN  = $(INSTFLAGS_FILE) -m $(INST_PERM_BIN)  $(INST_FLAGS)
INST_FLAGS_DIR  = $(INSTFLAGS_DIR)  -m $(INST_PERM_BIN)  $(INST_FLAGS) -d


#
# Installation directory tree (default locations under PREFIX)
#

ifndef BININSTDIR
  BININSTDIR = $(PREFIX)/bin
endif
ifndef MANINSTDIR
  MANINSTDIR = $(PREFIX)/share/man
endif
ifndef LIBINSTDIR
  LIBINSTDIR = $(PREFIX)/lib
endif
ifndef INCINSTDIR
  INCINSTDIR = $(PREFIX)/include
endif

# Additional copy of the default directories, to allow a divergence between compile time
# and runtime values. This split is necessary e.g. when packaging the cwb for a different
# system.
# The FINAL* paths should be used whenever the actual installed system-location is needed.
# For example when building an RPM package, PREFIX needs to point to the RPM %{buildroot}
# while the paths in the compiled binaries need to point to their actual system locations
# i.e. /usr
# This is a specific fix to stop temporary packaging paths from appearing in cwb-config
# and messing with Perl modules.

ifndef FINALPREFIX
  FINALPREFIX = $(PREFIX)
endif

ifndef FINALBININSTDIR
  FINALBININSTDIR = $(FINALPREFIX)/bin
endif
ifndef FINALMANINSTDIR
  FINALMANINSTDIR = $(FINALPREFIX)/share/man
endif
ifndef FINALLIBINSTDIR
  FINALLIBINSTDIR = $(FINALPREFIX)/lib
endif
ifndef FINALINCINSTDIR
  FINALINCINSTDIR = $(FINALPREFIX)/include
endif

ifndef DEFAULT_REGISTRY
  DEFAULT_REGISTRY = $(FINALPREFIX)/share/cwb/registry
endif

# Note that the above variables are *not* used when building a Windows release, as there,
# the PREFIX is determined at install time (as per Windows general practice). 


#
# Variables for release bundle creation 
#

# CPU architecture and operating system (only used for naming binary releases)
ifndef RELEASE_ARCH
  RELEASE_ARCH = $(shell uname -m)
endif
ifndef RELEASE_OS
  RELEASE_OS = $(shell uname -s)-$(shell uname -r)
endif

# directory and path for binary release
RELEASE_NAME = cwb-$(VERSION)-$(RELEASE_OS)-$(RELEASE_ARCH)
RELEASE_DIR = $(TOP)/build/$(RELEASE_NAME)

# directory and path for source code release
TARBALL_NAME = cwb-$(VERSION)-src
TARBALL_DIR = $(TOP)/build/$(TARBALL_NAME)

# directory for debian release
DEBIAN_NAME = cwb-$(VERSION)
DEBIAN_TARBALL_NAME = cwb_$(VERSION).orig.tar.gz
DEBIAN_DIR = $(TOP)/build/$(DEBIAN_NAME)

# commands / filenames used by make release
ifndef WINDOWS
  RELEASE_COMPRESSED_FILENAME = "$(RELEASE_NAME).tar.gz"
  TARBALL_COMPRESSED_FILENAME = "$(TARBALL_NAME).tar.gz"
  COMPRESS_COMMAND = $(TAR) cfz
else
  RELEASE_COMPRESSED_FILENAME = "$(RELEASE_NAME).zip"
  TARBALL_COMPRESSED_FILENAME = "$(TARBALL_NAME).zip"
  COMPRESS_COMMAND = $(ZIP)
endif


#
# Set up compiler and linker flags
#

# do we want to use the Valgrind-ready version of DEBUG_FLAGS and CFLAGS?
ifdef VALGRIND_READY
  DEBUG_FLAGS := -Og -g 
  CFLAGS := $(shell echo "$(CFLAGS)" | perl -pe 's/(^|\s)-O\d\s/ /g')
# it's reasonable, for this developer-only affordance, to assume we've got Perl.
endif

CFLAGS += $(DEBUG_FLAGS) $(SITE_CFLAGS)
LDFLAGS += $(DEBUG_FLAGS) $(SITE_LDFLAGS)

# termcap/curses support is activated by setting TERMCAP_LIBS
ifdef TERMCAP_LIBS
  CFLAGS += -DUSE_TERMCAP
endif

# same for GNU Readline library support
ifdef READLINE_LIBS
  CFLAGS += -DUSE_READLINE
endif

# Glib and PCRE : (a) header file info (b) linker flags for libraries used by the CL
#   (a) are used when compiling any .c files
#   (b) are added to linking commands for all programs
#
# FIRST, static linking for Windows release. This causes libcl.dll and all .exe to contain
# the library code that we need. So the code objects are (should be) self-contained.
ifdef WINDOWS_STATIC
  # we need special treatment of the config calls for static linking on Win;
  # otherwise, it's fine to fall through (whether Win/*nix)
  ifndef PCRE_DEFINES
    PCRE_DEFINES := -DPCRE_STATIC $(shell pcre2-config --cflags)
  endif

  ifndef GLIB_DEFINES
    GLIB_DEFINES := -DGLIB_STATIC_COMPILATION $(shell pkg-config --static --cflags glib-2.0)
  endif
  
  # let's also pre-define the corresponding LIBS variables
  ifndef PCRE_LIBS
    PCRE_LIBS := -DPCRE_STATIC $(shell pcre2-config --libs8)
  endif

  ifndef GLIB_LIBS
    # the GLIB_STATIC_COMPILATION prevents Glib functions from being "not found"
    # Glib needs -liconv to be stated for static linking, yet pkg-config doesn't produce it!
    GLIB_LIBS := -DGLIB_STATIC_COMPILATION $(shell pkg-config --static --libs glib-2.0) -liconv
  endif
endif

# SECOND, any other case than static linking for Windows release
ifndef PCRE_DEFINES
  PCRE_DEFINES := $(shell pcre2-config --cflags)
endif

ifndef GLIB_DEFINES
  GLIB_DEFINES := $(shell pkg-config --cflags glib-2.0)
endif

ifndef PCRE_LIBS
  PCRE_LIBS := $(shell pcre2-config --libs8)
endif

ifndef GLIB_LIBS
  GLIB_LIBS := $(shell pkg-config --libs glib-2.0)
endif

# define macro variables for some global settings
INTERNAL_DEFINES = -DCWB_REGISTRY_DEFAULT_PATH=\""$(DEFAULT_REGISTRY)"\" -DCOMPILE_DATE=\"$(COMPILE_DATE)\" -DCWB_VERSION=\"$(VERSION)\"

# path to locally compiled CL library and linker command:
#
# linking cqp and cwb-* against the .so rather than the .a is the more modern way, and preferred on Linux,
# but possibly not on all other platforms. So the fallback is still static.
ifndef LIBCL_TYPE
  LIBCL_TYPE = static
endif
ifeq ($(LIBCL_TYPE),shared)
  ifdef WINDOWS
    # we don't need to mention the libcl.lib file here, even though the variable LIBCL_SHAREDLIB mentions both .dll and .lib. 
    LIBCL_PATH = $(TOP)/cl/libcl.dll
  else
    LIBCL_PATH = $(TOP)/cl/$(LIBCL_SHAREDLIB)
  endif
else
  LIBCL_PATH = $(TOP)/cl/libcl.a
endif

# Flags to set run-time search path when linking against shared library.
# Can be overridden by platform configuration files, use something like
#   LIBCL_RPATH = $(DONT_SET_LIBCL_RPATH)
# to disable the built-in logic.
ifndef LIBCL_RPATH
  # only set rpath for non-default paths, since hard coding the rpath to /usr/lib 
  # can cause problems with the default linker settings on multilib distros like Debian
  ifeq ($(FINALLIBINSTDIR),/usr/lib)
    LIBCL_RPATH =
  else
    # this seems to be the most widely-supported notation for passing linker flags
    LIBCL_RPATH = -Xlinker -rpath -Xlinker $(FINALLIBINSTDIR)
    # LIBCL_RPATH = -Wl,-rpath,$(FINALLIBINSTDIR)  # works on GCC and many other compilers
  endif
endif

# The options for linking CQP and utilities against CL provide a relative path to the appropriate
# libcl file for static/dynamic linking, and an optional run-time search path for dynamic libraries
ifeq ($(LIBCL_TYPE),shared)
  CL_LIBS = $(LIBCL_PATH) $(LIBCL_RPATH)
else
  CL_LIBS = $(LIBCL_PATH)
endif

LDFLAGS_LIBS = $(PCRE_LIBS) $(GLIB_LIBS) 

# complete sets of compiler and linker flags (allows easy specification of specific build rules)
CFLAGS_ALL = $(CFLAGS) $(INTERNAL_DEFINES) $(GLIB_DEFINES) $(PCRE_DEFINES) $(READLINE_DEFINES) $(TERMCAP_DEFINES)
DEPEND_CFLAGS_ALL = $(DEPEND_CLAGS) $(INTERNAL_DEFINES) $(GLIB_DEFINES) $(PCRE_DEFINES) $(READLINE_DEFINES) $(TERMCAP_DEFINES)
LDFLAGS_ALL = $(LDFLAGS) $(PCRE_LIBS) $(GLIB_LIBS) 

# readline and termcap libraries are only needed for building CQP
LDFLAGS_CQP = $(READLINE_LIBS) $(TERMCAP_LIBS)

# extra libraries to be included in the libcl DLL for a self-contained binary package
ifndef LIBCL_EXTRA_LIBS
  LIBCL_EXTRA_LIBS =
endif

# extra libraries to be copied into the release bundle for a self-contained binary package
ifndef RELEASE_EXTRA_LIBS
  RELEASE_EXTRA_LIBS =
endif



# 
# A gesture in the direction of user-friendliness:
# 
# Use .SILENT to turn on or off the grizzly bits of what displays on screen.
# Ultimately depends on FULL_MESSAGES, which can be set in the config file or on the command line.
#  
ifdef FULL_MESSAGES
  FULL_OUTPUT_VAR = .no_silence_allowed
else
  MAKEFLAGS += --no-print-directory
  FULL_OUTPUT_VAR =
endif

# the .SILENT target is added to every makefile by this mechanism.
$(FULL_OUTPUT_VAR).SILENT :


#
# Chapter for manpages (all our manfiles are for command-line executables, so they all go in chapter 1)
#

MANEXT = 1


#
# General .c -> .o build rule
#

%.o : %.c
	@$(ECHO) "    .... compiling source file" $< 
	$(RM) $@
	$(CC) -c  -o $@ $(CFLAGS_ALL) $<
