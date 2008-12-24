#!/usr/bin/perl

use lib "$ENV{GLEDSYS}/perllib";
use Gled_Installer;

Gled_Installer::parse_opts();

my ($d, $s, @files) = @ARGV;

if ($Gled_Installer::HELP or not defined $d or not defined $s or $#files < 0)
{
  print "usage: gled_install [options] <destination_dir> <source dir> <files>\n" .
    Gled_Installer::option_string();
  exit 0;
}

if (not -d $s)
{
  print STDERR "Source directory '$s' does not exist.\n";
  exit 1;
}

Gled_Installer::install($d, $s, @files);
