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


# **********************************************************
# * Edit this file to configure CWB for your system *
# **********************************************************

# Optionally read settings from a separate configuration file "local_config.mk".
# This is recommended when working in an SVN sandbox in order to avoid conflicts
# with version control.
-include $(TOP)/local_config.mk

# 
# PLATFORM-SPECIFIC CONFIGURATION (OS and CPU type)
#
# Pre-defined platform configuration files (nesting shows inheritance among the different configs):
#
#       unix                generic Unix + GCC configuration (should work on most Unix platforms)
#       linux               Linux (generic)
#           linux-native        - Linux default build (usually 64-bit), natively tuned (recommended)
#           linux-64            - configuration for 64-bit CPUs
#           linux-32            - configuration for 32-bit CPUs (used for cross-compilation; now --deprecated--)
#           linux-opteron       - with optimization for AMD Opteron processor
#       darwin              MacOS / Darwin build for default architecture, with prerequisites installed by user
#           darwin-brew         - Intel 64-bit, natively tuned, with prerequisites installed with HomeBrew (recommended)
#           darwin-brew-m1          - same for ARM 64-bit (M1 and other Apple Silicon, recommended)
#           darwin-release      - Intel 64-bit for self-contained binary release, with static prerequisites in assets/
#           darwin-release-m1       - same for ARM 64-bit
#           darwin-brew-release - Intel 64-bit for self-contained binary release, with prerequisites from HomeBrew  --dispreferred--
#           darwin-brew-release-m1  - same for ARM 64-bit  --dispreferred--
#           darwin-64           - Intel 64-bit, natively tuned, prerequisites installed by user  --deprecated--
#           darwin-universal    - universal 64-bit/32-bit build on Mac OS X 10.6-10.13  --deprecated--
#           darwin-port         - generic build, prerequisites installed with MacPorts  --deprecated--
#       solaris             SUN Solaris 8 for SPARC CPU  --unsupported--
#       cygwin              Win32 build using Cygwin emulation layer  --deprecated--
#       win64-msys2         Win64 build using the MSYS2 environment.
#       homebrew-formula    configuration for HomeBrew package manager
#                               - set HOMEBREW_ROOT to base of HomeBrew tree on command line
#
ifndef PLATFORM
PLATFORM=linux-native
endif
include $(TOP)/config/platform/$(PLATFORM)

#
# SITE-SPECIFIC CONFIGURATION (installation path and other local settings)
#
# Pre-defined site configuration files (nesting shows inheritance among the different configs):
#
#       standard            standard configuration for local installation of a build from source
#           beta-install        - install into separate tree /opt/cwb/cwb-<VERSION> (for beta testing)
#       classic             "classic" configuration (like CWB v2.2, uses /corpora/c1/registry)
#       binary-release      build binary package for release (use with "make release"); 
#                           requires an appropriate, compatible PLATFORM to be specified. 
#           darwin-release      - for MacOS
#           linux-release       - for Linux; independent use of this config is  --defunct--
#                                 but it is retained so it can be inherited from.
#               linux-deb           - as an APT-compatible .deb package (for Debian/Ubuntu/etc.)
#           windows-release     - for Windows binaries to be bundled into a zip
#                                 (use "standard" to install within MSYS2 itself)
#           solaris-release     - for SUN Solaris 2.x  --defunct--
#       cygwin              Cygwin on Win32 configuration (experimental)  --deprecated--
#       homebrew-formula    configuration for HomeBrew package manager
#                               - set HOMEBREW_ROOT to base of HomeBrew tree on command line
#                               - must set PREFIX to formula's keg directory
#
ifndef SITE
SITE=standard
endif
include $(TOP)/config/site/$(SITE)

#
# MANUAL CONFIGURATION (override individual platform and site settings)
#
# Manual configuration should only be used for testing or for one-off installation.
# If you intend to install further CWB releases on the same machine, it is recommended
# that you write your own configuration files (which can be stored outside the CWB
# source tree).  See INSTALL for more information.
#
# To override individual settings, uncomment and edit one or more of the assignments
# below.  The values shown are the "typical" defaults, but may be changed in the 
# platform and site configuration files you selected.
#


## SPECIAL: enable full output
## ---------------------------
## Normally, the complex output from the build process is hidden.
## But if you would like to see ALL the output, you can uncomment the line below.
# FULL_MESSAGES = 1




## Directory tree for software installation
# PREFIX = /usr/local

## Individual installation paths can be overridden
# BININSTDIR = $(PREFIX)/bin
# LIBINSTDIR = $(PREFIX)/lib
# INCINSTDIR = $(PREFIX)/include
# MANINSTDIR = $(PREFIX)/share/man

## Access permissions for installed files, optionally owner and group settings
# INST_PERM = 644
# INST_USER = ???
# INST_GROUP = ???

## Default registry for corpus declarations
# DEFAULT_REGISTRY = $(PREFIX)/share/cwb/registry

## CPU architecture and operating system for naming binary releases
# RELEASE_ARCH = ???  # e.g. i386 or x86_64
# RELEASE_OS = ???    # e.g. linux-2.6 or osx-10.4

## C compiler to use (GCC is highly recommended, others may not work)
# CC = gcc

## Override options for C compiler and linker (complete override)
# CFLAGS = -O2 -Wall
# LDFLAGS = -lm

## Include debugging information in binaries (for developers only, not enabled by default)
# DEBUG_FLAGS = -g
## Preparing executables for use in Valgrind requires more complex manipulation of the gcc flags than just -g.
## Define VALGRIND_READY, and they'll be set up at time of make. 
# VALGRIND_READY = 

## Side-specific options are added to the standard CFLAGS and LDFLAGS (e.g. additional paths)
# SITE_CFLAGS =
# SITE_LDFLAGS =


#
# The following settings will only need to be changed in very rare cases.  If necessary, 
# they are usually set in the platform configuration file to work around OS deficiencies.
#
# When (cross-)compiling for Windows with MinGW, most of these settings are ignored
# and unconditionally replaced by hard-coded defaults. See file INSTALL-WIN for details.
#

## Some platforms require special libraries for socket/internet functionality 
# NETWORK_LIBS =

## CQP requires the termcap or ncurses library for text highlighting (setting TERMCAP_LIBS activates highlighting)
# TERMCAP_LIBS =
# TERMCAP_DEFINES = 

## GNU Readline library for command-line editing (optional)
# READLINE_LIBS = -L<path_to_readline_libs> -lreadline -lhistory
# READLINE_DEFINES = -I<path_to_readline_headers>

## GLIB2 for platform-independent support functions
# GLIB_LIBS = -L<path_to_glib_libs> -lglib-2.0
# GLIB_DEFINES = -I<path_to_glib_headers>

## PCRE regular expression library (PCRE2 has now replaced PCRE 1; we need the 8-bit-code-unit library)
# PCRE_LIBS = -L<path_to_pcre_libs> -lpcre2-8
# PCRE_DEFINES = -I<path_to_pcre_headers>

## CWB uses Flex/Bison for parsing registry files and CQP commands
# YACC = bison -d -t
# LEX = flex -8

## GNU-compatible install program (defaults to included shell script)
# INSTALL = $(TOP)/instutils/install.sh

## Sometimes, extra install flags are needed for files or directories (e.g. preserve modification time on OS X)
# INSTFLAGS_FILE = ???
# INSTFLAGS_DIR = ???

## Make index of symbols in source code for Emacs editor (usually etags or ctags -e)
# ETAGS = ???

## Update dependencies between source code files (flags depend on C compiler being used)
# DEPEND = gcc -MM -MG

## In the unlikely event that "date" does not work properly, or if you want to lie about the date
# COMPILE_DATE = $(shell date)

## CFLAGS and LDFLAGS for linking against binary releases
# RELEASE_CFLAGS = 
# RELEASE_LDFLAGS = -l cl

## Extra libraries to be linked into the libcl DLL (for binary releases)
# LIBCL_EXTRA_LIBS =


#
# ***** Do NOT edit anything below this point! *****
#

# load standard makefile settings (don't edit this)
include $(TOP)/definitions.mk
