#!/usr/bin/perl

use lib "$ENV{GLEDSYS}/perllib";
use Gled_Installer;

my ($d, $s, @dirs) = @ARGV;

unless(-d $d and -d $s and $#dirs >= 0) {
  print "usage: gled_install <destination_dir> <source dir> <subdirs of source dir>\n";
  exit 1;
}

Gled_Installer::install_dirs($d, $s, @dirs);
