#!/usr/bin/perl

use lib "$ENV{GLEDSYS}/perllib";
use Gled_Installer;

Gled_Installer::parse_opts();

my ($d, $s) = @ARGV;

if ($Gled_Installer::HELP or not defined $d or not defined $s)
{
  print "usage: gled_uninstall_files [options] <destination_dir> <source dir> <subdirs of source dir>\n" .
    Gled_Installer::option_string();
  exit 1;
}

unless (-d $d)
{
  print STDERR "Destination directory '$d' does not exist.\n";
  exit 1;
}

unless (-d $s)
{
  print STDERR "Source directory '$s' does not exist.\n";
  exit 1;
}

Gled_Installer::uninstall_files($d, $s);
