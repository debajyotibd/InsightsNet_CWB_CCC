#!/usr/bin/perl
# -*-cperl-*-

use strict;
use warnings;

my $Force = 0;
if (@ARGV >= 1 && $ARGV[0] =~ /^--?y(es)?$/i) {
  $Force = 1;
  shift @ARGV;
}

die <<USAGE unless @ARGV == 1;
Usage:  perl uninstall-cwb.perl [-y | --yes] <prefix>

Uninstall CWB v3.5 from specified directory tree <prefix>.
The script will display all files to be removed and then
ask for confirmation (unless overridden with the -y option).
USAGE

my $Prefix = shift @ARGV;
die "Error: directory tree $Prefix doesn't exist.\n" unless -d $Prefix;


my @Files = (); # collect files and directories to be deleted
my @Dirs = ();
sub add_files {
  foreach (@_) {
    push @Files, $_ if -f $_;
  }
}

## CWB binaries and manpages
my @binaries = qw(cqp cqpcl cqpserver cwb-align cwb-align-decode cwb-align-encode cwb-align-show cwb-atoi cwb-compress-rdx cwb-config cwb-decode cwb-decode-nqrfile cwb-describe-corpus cwb-encode cwb-huffcode cwb-itoa cwb-lexdecode cwb-makeall cwb-s-decode cwb-s-encode cwb-scan-corpus);
add_files(map {"$Prefix/bin/$_"} @binaries);
add_files(map {"$Prefix/bin/$_.exe"} @binaries);    # on Windows
add_files(map {"$Prefix/man/man1/$_.1"} @binaries); # manpages
add_files(map {"$Prefix/share/man/man1/$_.1"} @binaries); # alternative location


## headers
add_files("$Prefix/include/cwb/cl.h", "$Prefix/include/cwb/cqi.h", "$Prefix/include/cqi.h");
push @Dirs, "$Prefix/include/cwb" if -d "$Prefix/include/cwb";

## libraries
add_files(glob "$Prefix/lib/libcl.*");

unless (@Files) {
  die "Error: nothing to delete.\n" unless $Force;
  print "CWB installation not found in $Prefix (skipped).\n";
  exit 0;
}

## show what files will be deleted
print "The following files will be deleted:\n";
foreach (@Files) {
  print "\t$_\n";
}
foreach (@Dirs) {
  print "\t$_/\n";  
}
print "\n";

## ask user for confirmation
unless ($Force) {
  print "Enter 'yes' to delete these files: ";
  my $answer = <STDIN>;
  chomp $answer;
  die "Aborted.\n" unless lc($answer) eq "yes";
}

## delete the files
foreach (@Files) {
  unlink $_ || die "Error: unable to delete file $_ because $!";
}
foreach (@Dirs) {
  rmdir $_ || die "Error: unable to remove directory $_ because $!";
}

printf "%d files deleted.\n", scalar @Files;
