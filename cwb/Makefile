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


##  This is the main Makefile for the CWB. 


#
#  Welcome to the IMS Open Corpus Workbench.  See "INSTALL" for detailed
#  installation instructions.  For a quick start, select appropriate PLATFORM
#  and SITE configurations in "config.mk", then build with "make all".
#  
#  The following make targets are available:
#
#  all          compile the Corpus Library, CQP, command-line utilities and man pages
#  install      install CL, CQP, utilities and man pages
#  release      create a release file (binaries and documentation)
#  tarball      create a source tarball
#  deb-release  create Debian source and binary packages (requires a debian environment)
#  clean        clean up in all subdirectories 
#  realclean    also delete automatically generated parsers, dependencies, etc.
#
#  size         check size of sourcecode (in lines)
#  depend       update dependencies (only necessary after major changes)
#  tags         create an emacs ETAGS file
#
#  cl           to compile the Corpus Library only
#  cqp          to compile CQP only (inc. the elements in the separate CQi directory)
#  utils        to compile the utilities only
#  man          to update the manpages from the .pod sources
#  doxygen      to update the HTML code documentation (requires doxygen to be installed)
#  docset       to update the MacOS docset (requires doxygen to be installed)
#  

TOP = $(shell pwd)
include $(TOP)/config.mk

# targets for external libraries; currently not used (Glib and PCRE2 assumed to be on the system already)
ifdef WINDOWS
  EXTERNALS = 
else
  EXTERNALS = 
endif

SUBDIRS = cl cqp utils man instutils    # subdirectories that have their own makefiles
SRCDIRS = cl cqp utils CQi              # subdirectories containing C source code


.PHONY: clean realclean depend all test install uninstall release mingw-libgnurx-2.5.1 cl cqp utils man instutils tags size

default:
	@$(ECHO) "Please type one of the following:"
	@$(ECHO) ""
	@$(ECHO) "  make clean        remove binaries and temporary files"
	@$(ECHO) "  make realclean    delete all automatically generated files"
	@$(ECHO) "  make depend       update dependencies"
	@$(ECHO) "  make all          build CWB libaries, programs and man pages"
	@$(ECHO) "  make install      install CWB into chosen location"
	@$(ECHO) "  make release      create binary release in build/ directory"
	@$(ECHO) "  make tarball      create source code release in build/ directory"
	@$(ECHO) "  make deb-release  create Debian source and binary packages in "
	@$(ECHO) "                        build/ directory (requires Debian environment)"
	@$(ECHO) ""
	@$(ECHO) "  make cl           build low-level corpus library (CL)"
	@$(ECHO) "  make cqp          build CQP query processor"
	@$(ECHO) "  make utils        build command-line utilities"
	@$(ECHO) "  make man          update manpages from POD sources"
	@$(ECHO) "  make doxygen      update HTML code documentation"
	@$(ECHO) "  make docset       create MacOS docset (see README)"
	@$(ECHO) "                        (docset/doxygen targets require Doxygen program)"
	@$(ECHO) "  make instutils    configure installation scripts"
	@$(ECHO) "  make tags         generate symbol index for GNU Emacs"
	@$(ECHO) "  make size         check size of CWB source code"

all: $(EXTERNALS)
	@$(ECHO) "--------------------------------- BUILDING CORPUS LIBRARY (CL)"
	$(MAKE) -C cl
	@$(ECHO) "--------------------------------- BUILDING CQP"
	$(MAKE) -C cqp
	@$(ECHO) "--------------------------------- BUILDING COMMAND-LINE UTILITIES"
	$(MAKE) -C utils
	@$(ECHO) "--------------------------------- BUILDING MANPAGES"
	$(MAKE) -C man

cl:
	@$(ECHO) "--------------------------------- BUILDING CORPUS LIBRARY (CL)"
	$(MAKE) -C cl 

cqp:
	@$(ECHO) "--------------------------------- BUILDING CQP"
	$(MAKE) -C cqp 

utils:
	@$(ECHO) "--------------------------------- BUILDING COMMAND-LINE UTILITIES"
	$(MAKE) -C utils 

man:
	@$(ECHO) "--------------------------------- BUILDING MANPAGES"
	$(MAKE) -C man 

doxygen:
	@$(ECHO) "--------------------------------- BUILDING CODE DOCUMENTATION"
	-$(RM) -rf doc/html
	doxygen doc/doxygen-config
	(cd doc && perl textile2html.perl)

docset:
	@$(ECHO) "--------------------------------- BUILDING CODE DOCSET (MacOS)"
	-$(RM) -rf doc/docset_html build/net.sourceforge.cwb.CWB3-Code.docset
	doxygen doc/doxygen-config-docset && \
	perl -i -lpe '$$_="DESTDIR=../../build" if /^DESTDIR=/; $$_="" if /^XCODE_INSTALL/; s{^(\s+).*/(docsetutil index)}{$$1$$2};' doc/docset_html/Makefile && \
	(cd doc/docset_html && make install) && \
	perl -i -lpe '$$_ .= "\n\t<key>dashIndexFilePath</key>\n\t<string>index.html</string>" if /^\s*<dict>/; s{(<string>)doxygen(</string>)}{$$1cwb$$2}' build/net.sourceforge.cwb.CWB3-Code.docset/Contents/Info.plist && \
	$(RM) -rf doc/docset_html && \
	$(ECHO) "Created MacOS docset in build/net.sourceforge.cwb.CWB3-Code.docset/"

instutils:
	@$(ECHO) "--------------------------------- CONFIGURING INSTUTILS"
	$(MAKE) -C instutils

size:
	@$(ECHO) "--------------------------------- MEASURING EXTENT OF THE CWB CODE"
	for i in cl cqp utils ; do $(MAKE) -C $$i size ; done;

depend:
	@$(ECHO) "--------------------------------- UPDATING ALL DEPENDENCIES"
	for i in $(SUBDIRS) ; do $(MAKE) -C $$i depend ; done;

clean:
	@$(ECHO) "--------------------------------- PERFORMING TYPICAL CLEANUP"
	for i in $(SUBDIRS) $(EXTERNALS); do if [ -f "$$i/Makefile" ]; then $(MAKE) -C $$i clean; fi; done;
	-$(RM) *~ config/*/*~ CQi/*~ CQi/*.o technical/*~ TAGS
	-$(RM) -rf build/* doc/docset_html

realclean: clean
	@$(ECHO) "--------------------------------- PERFORMING EXTRA CLEANUP"
	for i in $(SUBDIRS) ; do $(MAKE) -C $$i realclean; done;

install:
ifdef WINDOWS_RELEASE_EXT
	@$(ECHO) "ERROR: install operation is not supported in 'Windows-Release' mode!"
else
	@$(ECHO) "--------------------------------- INSTALLING CORPUS WORKBENCH"
	$(MAKE) -C cl install   # we're not using a for-loop, to make sure we abort on error
	$(MAKE) -C cqp install
	$(MAKE) -C utils install
	$(MAKE) -C man install
	$(MAKE) -C instutils all
	$(MAKE) -C instutils install
endif

release:
	@$(ECHO) "--------------------------------- BUILDING RELEASE BUNDLE"
	if [ -d "$(RELEASE_DIR)" ]; then $(RM) -r "$(RELEASE_DIR)"; fi
	$(INSTALL) $(INST_FLAGS_DIR) "$(RELEASE_DIR)"
	$(MAKE) -C cl release
	$(MAKE) -C cqp release
	$(MAKE) -C utils release
	$(MAKE) -C man release
	$(MAKE) -C instutils all
	$(MAKE) -C instutils release
	# in case we need additional dynamic libraries to be included in the binary release package
	# (currently not in use, as libcl.dll / libcl.dylib already include all required functions)
	for i in $(RELEASE_EXTRA_LIBS); do $(INSTALL) $(INST_FLAGS_DATA) "$$i" "$(RELEASE_DIR)/lib"; done
	if [ -f "$(RELEASE_COMPRESSED_FILENAME)" ]; then $(RM) "$(RELEASE_COMPRESSED_FILENAME)"; fi
	(cd build/ && $(COMPRESS_COMMAND) "$(RELEASE_COMPRESSED_FILENAME)" "$(RELEASE_NAME)")
	@$(ECHO) "==> CREATED BINARY RELEASE in build/$(RELEASE_COMPRESSED_FILENAME)"

tarball: realclean
	@$(ECHO) "--------------------------------- BUILDING SOURCE RELEASE"
	$(MAKE) -C cl grammar
	$(MAKE) -C cqp grammar
	$(MAKE) man
	if [ -d "$(TARBALL_DIR)" ]; then $(RM) -r "$(TARBALL_DIR)"; fi
	$(INSTALL) $(INST_FLAGS_DIR) "$(TARBALL_DIR)"
	$(CP) -p AUTHORS CHANGES COPYING INSTALL INSTALL-MACOS INSTALL-WIN README "$(TARBALL_DIR)"
	$(CP) -p config.mk definitions.mk Makefile "$(TARBALL_DIR)"
	$(CP) -pR CQi cl config cqp doc install-scripts instutils man utils "$(TARBALL_DIR)"
	$(INSTALL) $(INST_FLAGS_DIR) "$(TARBALL_DIR)/build"
	if [ -f "$(TARBALL_COMPRESSED_FILENAME)" ]; then $(RM) "$(TARBALL_COMPRESSED_FILENAME)"; fi
	(cd build/ && $(COMPRESS_COMMAND) "$(TARBALL_COMPRESSED_FILENAME)" "$(TARBALL_NAME)")
	@$(ECHO) "==> CREATED SOURCE RELEASE in build/$(TARBALL_COMPRESSED_FILENAME)"

deb-release: realclean
	# this involves copying all the source files, similar to tarball creation, but with some extra steps
	# NB. package "debscripts" must be installed, as must "debhelper", "fakeroot"
	@$(ECHO) "--------------------------------- BUILDING DEB SOURCE AND BINARY RELEASE"
	$(MAKE) -C cl grammar
	$(MAKE) -C cqp grammar
	$(MAKE) man
	if [ -d "$(DEBIAN_DIR)" ]; then $(RM) -rf "$(DEBIAN_DIR)"; fi
	$(INSTALL) $(INST_FLAGS_DIR) "$(DEBIAN_DIR)"
	$(CP) -p AUTHORS CHANGES COPYING INSTALL INSTALL-MACOS INSTALL-WIN README "$(DEBIAN_DIR)"
	$(CP) -p config.mk definitions.mk Makefile "$(DEBIAN_DIR)"
	$(CP) -pR CQi cl config cqp doc install-scripts instutils man utils "$(DEBIAN_DIR)"
	$(INSTALL) $(INST_FLAGS_DIR) "$(DEBIAN_DIR)/build"
	# the important debian directory with all the packaging stuff
	$(CP) -pR packaging/debian "$(DEBIAN_DIR)"

	# inserting references to the right build config files
	$(SED) -iE "s/PLATFORM=.*/PLATFORM=linux-64/" $(DEBIAN_DIR)/config.mk
	$(SED) -iE "s/SITE=.*/SITE=linux-deb/" $(DEBIAN_DIR)/config.mk

	if [ -f "$(DEBIAN_TARBALL_NAME)" ]; then $(RM) "$(DEBIAN_TARBALL_NAME)"; fi
	(cd $(DEBIAN_DIR) && $(COMPRESS_COMMAND) "../$(DEBIAN_TARBALL_NAME)" .)

	# building the debian package
	(cd $(DEBIAN_DIR) && debuild -us -uc --lintian-opts --no-lintian)
	@$(ECHO) "==> CREATED DEB SOURCE AND BINARY PACKAGES in build/"

tags:
	@$(ECHO) "--------------------------------- GENERATING 'TAGS' FILES"
	rm -f TAGS
	for i in $(SRCDIRS) ; do  $(ETAGS) -a -o TAGS $$i/*.[chly]; done
	# NB "-a" option is required because we build the TAGS file incrementally

##
## Last few targets: hints for actions that don't exist. 
##

uninstall:
	@$(ECHO) "Uninstall is not supported via 'make'; try 'perl instutils/uninstall-cwb.perl <prefix>' instead."

test:
	@$(ECHO) "No self-tests available in the CWB core at the moment; try the test suite in the CWB-Perl distribution."

rpm-release:
	@$(ECHO) "Creating an RPM release package is not automatable via 'make'; please see 'INSTALL' for more info on RPM creation." 

