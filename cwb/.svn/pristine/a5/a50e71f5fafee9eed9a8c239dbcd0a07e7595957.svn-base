#!/usr/bin/perl
# -*-cperl-*-

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

use strict;
use warnings;
use FindBin qw($RealBin);

sub usage {
  print "Usage:  cwb-config <flag> [...]\n";
  print "Available configuration flags:\n";
  print "   --version, -v    installed CWB version\n";
  print "   --prefix, -p     CWB installation prefix (may have been overridden)\n";
  print "   --bindir, -b     install location of binaries\n";
  print "   --libdir, -l     install location of CL library\n";
  print "   --incdir, -i     install location of C header files\n";
  print "   --mandir, -m     install location of manual pages\n";
  print "   --cflags, -I     compiler flags for linking against CL library\n";
  print "   --ldflags, -L    linker flags for linking against CL library\n";
  print "   --registry, -r   effective registry directory or directories\n";
  print "   --default-registry, -d  default registry directory compiled into CWB\n";
  exit 2;
}

usage() unless @ARGV;

## determine root directory of binary package
my $Prefix = $RealBin;
$Prefix =~ s{/bin/?$}{} or die "Error: your CWB binary package appears to be damaged, please re-install\n";
# $Prefix =~ s{/instutils/?$}{} or die "INTERNAL ERROR"; # FIXME
die "Error: your CWB binary package appears to be damaged, please re-install\n" unless -f "$Prefix/README";

my $DefaultRegistry = "/usr/local/share/cwb/registry"; # default path for binary releases on all POSIX platforms

## process command-line flags and print requested information
foreach (@ARGV) {
  if (/^--?h(elp)?$/) {
    usage();
  }
  elsif (/^--?v(ersion)?$/) {
    my $version_info = `'$Prefix/bin/cqp' -v`; 
    $version_info =~ /Version:\s+(\d+[.\w]+)\s*$/ or die "Error: can't execute CQP, your CWB binary package appears to be damaged\n";
    print "$1\n";
  }
  elsif (/^--?p(refix)?$/) {
    print "$Prefix\n";
  }
  elsif (/^--?b(indir)?$/) {
    print "$Prefix/bin\n";
  }
  elsif (/^--?l(ibdir)?$/) {
    print "$Prefix/lib\n";
  }
  elsif (/^--?i(ncdir)?$/) {
    print "$Prefix/include\n";
  }
  elsif (/^--?m(andir)?$/) {
    print "$Prefix/man\n";
  }
  elsif (/^--?(I|cflags)$/) {
    print "-I'$Prefix/include'\n";
  }
  elsif (/^--?(L|ldflags)$/) {
    print "-Xlinker -rpath -Xlinker '$Prefix/lib' -L'$Prefix/lib' -lcl\n";
  }
  elsif (/^--?r(egistry)?$/) {
    my $registry = $ENV{CORPUS_REGISTRY} || $DefaultRegistry;
    print "$registry\n";
  }
  elsif (/^--?d(efault-registry)?$/) {
    print "$DefaultRegistry\n";
  }
  else {
    die "Error: unsupported flag '$_'\n";
  }
}
