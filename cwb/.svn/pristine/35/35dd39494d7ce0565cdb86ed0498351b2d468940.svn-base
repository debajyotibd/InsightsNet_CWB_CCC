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
##

## Read configuration settings and standard definitions
TOP = $(shell pwd)/..
include $(TOP)/config.mk

## The following targets are available:
#
#  all         force rebuild of cwb-config and/or the Windows installation script
#  install     install cwb-config program
#  uninstall   <currently not supported>
#  release     copy special cwb-config and other files to binary release directory
#  clean       <dummy target>
#  realclean   remove automatically generated files
#  depend      <dummy target>
#  size        <dummy target>
#
## This file is full of "ifndef..." because install/release setup is so different 
## on Windows. In all cases, the *nix version comes first, then the Win64 version.
## Some lines are repeated redundantly for clarity. The controlling Make variable
## is WINDOWS_RELEASE_EXT, which is defined in ../config/site/windows-release . 


############################################################ all
ifndef WINDOWS_RELEASE_EXT
all:
	if [ -f cwb-config ];      then $(RM) cwb-config;      fi
	$(MAKE) cwb-config
else
all:
	if [ -f install-cwb.bat ]; then $(RM) install-cwb.bat; fi
	$(MAKE) install-cwb.bat
endif


############################################################ cwb-config
ifndef WINDOWS_RELEASE_EXT
## Insert configuration information into cwb-config for direct installation
cwb-config:
	VERSION='$(VERSION)' PREFIX='$(FINALPREFIX)' BINDIR='$(FINALBININSTDIR)' LIBDIR='$(FINALLIBINSTDIR)' INCDIR='$(FINALINCINSTDIR)' MANDIR='$(FINALMANINSTDIR)' CFLAGS='-I$(FINALINCINSTDIR)' LDFLAGS='-L$(FINALLIBINSTDIR) -lcl $(LIB_REGEX) $(LDFLAGS_ALL)' REGISTRY='$(DEFAULT_REGISTRY)' sh cwb-config.in > cwb-config
	## NB: we have removed $(GLIB_DEFINES) from CFLAGS= above because Glib and PCRE headers shouldn't be needed to link against CL
endif
## Nothing at all is done for Windows, because there, cwb-config.bat is created at install time.


############################################################ install-cwb
## Configuration information for binary release (will be adapted to installation directories)
## NB: Flags are set independently from configuration used for CWB source configuration because
##     1) binary release should not need to link against 3rd-party libraries
##        (which may be installed in other location on end user's machine, if at all)
##     2) different compiler settings may be desired or required (e.g. for compiling Perl/Python API)
ifndef WINDOWS_RELEASE_EXT
install-cwb.sh:
	VERSION='$(VERSION)' PREFIX='$(FINALPREFIX)' REGISTRY='$(DEFAULT_REGISTRY)' MANEXT='$(MANEXT)' RELEASE_CFLAGS='$(RELEASE_CFLAGS)' RELEASE_LDFLAGS='$(RELEASE_LDFLAGS)' sh install-cwb.sh.in > install-cwb.sh
else
install-cwb.bat:
	VERSION='$(VERSION)' PREFIX='$(PREFIX)' DEFAULT_REGISTRY_LOCFILE='${DEFAULT_REGISTRY_LOCFILE}' sh install-cwb-win.bat.in > install-cwb.bat
endif

############################################################ install
ifndef WINDOWS_RELEASE_EXT
install: cwb-config
	if [ ! -d $(BININSTDIR) ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(BININSTDIR)"; fi
	$(INSTALL) $(INST_FLAGS_BIN) cwb-config "$(BININSTDIR)/cwb-config"
else
install:
	$(ECHO) "The make system is presently configured to produce installers for use *outside* MSYS2."
	$(ECHO) "To install inside MSYS2, try ''standard'' site configuration."
	$(ECHO) "To install outside MSYS2, try ''make release'' instead..."
endif


############################################################ release
ifndef WINDOWS_RELEASE_EXT
release:
	if [ ! -d "$(RELEASE_DIR)/bin" ]; then $(INSTALL) $(INST_FLAGS_DIR) "$(RELEASE_DIR)/bin"; fi;
	$(INSTALL) $(INST_FLAGS_BIN) cwb-config-release.perl "$(RELEASE_DIR)/bin/cwb-config"
	$(INSTALL) $(INST_FLAGS_DATA) INSTALL-release "$(RELEASE_DIR)/INSTALL"
	for i in README COPYING AUTHORS CHANGES; do $(INSTALL) $(INST_FLAGS_DATA) ../$$i "$(RELEASE_DIR)"; done
else
release: install-cwb.bat cwb-config-win.bat.in
	$(INSTALL) $(INST_FLAGS_DATA) INSTALL-release-win "$(RELEASE_DIR)/INSTALL.txt"; done
	for i in README COPYING AUTHORS CHANGES; do $(INSTALL) $(INST_FLAGS_DATA) ../$$i "$(RELEASE_DIR)/$$i.txt"; done
	for i in install-cwb.bat cwb-config-win.bat.in; do $(INSTALL) $(INST_FLAGS_BIN) $$i "$(RELEASE_DIR)"; done
  ifdef UNIX2DOS
	for i in INSTALL README COPYING AUTHORS CHANGES install-cwb.bat cwb-config-win.bat.in; do $(UNIX2DOS) "$(RELEASE_DIR)/$$i.txt"; done
  endif
endif


############################################################ clean
ifndef WINDOWS_RELEASE_EXT
clean:
	-$(RM) cwb-config install-cwb.sh *~
else
clean:
	-$(RM) install-cwb.bat *~
endif



############################################################ Remaining targets are the same for *nix and for Windows releases.

realclean: clean

depend:
	@$(ECHO) "         Nothing to be done for 'make depend' in directory 'instutils'"

size:
	@$(ECHO) "         Nothing to be done for 'make size' in directory 'instutils'"

