#!/usr/bin/perl
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

# -*-cperl-*-
## find Readline library on Mac OS X, preferring user-installed version (presumably true GNU Readline) over system library (Editline wrapper)

use strict;
use warnings;

die "Usage: find_readline.perl (--cflags|--ldflags|--check|--prefix)\n"
  unless @ARGV == 1 and $ARGV[0] =~ /^--(cflags|ldflags|check|prefix)$/;
my $mode = $1;

my @directories = (
  "/usr/local",                         # first choice: user library in /usr/local
  "/usr/local/opt/readline",            # HomeBrew keg-only install
  "/opt/homebrew/opt",                  # HomeBrew keg-only (on ARM hardware)
  "/opt/local",                         # MacPorts
  "/sw",                                # Fink
);

foreach my $dir (@directories) {
  my $inc_dir = "$dir/include";
  my $lib_dir = "$dir/lib";
  my $header1 = "$inc_dir/readline/readline.h";
  my $header2 = "$inc_dir/readline/history.h";
  my $lib1base = "$lib_dir/libreadline";
  my $lib2base = "$lib_dir/libhistory";
  if (
    -f $header1 && -f $header2 
    && (-f "${lib1base}.dylib" || -f "${lib1base}.a")
    && (-f "${lib2base}.dylib" || -f "${lib2base}.a")
  ) {
    ## found a locally installed version of GNU Readline
    if ($mode eq "check") {
      print "Auto-detected GNU Readline library in $dir/lib\n";
    }
    elsif ($mode eq "prefix") {
      print "$dir\n";
    }
    elsif ($mode eq "cflags") {
      print "-I'$inc_dir'\n";
    }
    else {
      print "-L'$lib_dir' -lreadline -lhistory\n";
    }
    exit 0;
  }
}

## fall back to system library (if available)
if (-f "/usr/include/readline/readline.h" && -f "/usr/include/readline/history.h" && -f "/usr/lib/libreadline.dylib") {    
  if ($mode eq "check") {
    print "Using system Editline library in /usr/lib\n";
  }
  elsif ($mode eq "prefix") {
    print "/usr\n";
  }
  elsif ($mode eq "cflags") {
    ## no special flags needed
  }
  else {
    print "-lreadline\n"; # Mac OS X provides no separate history library
  }
}
else {
  print "No Readline library found -- command-line editing has been disabled\n"
    if $mode eq "check";
  ## don't specify flags or libs --> readline support will be disabled
}
