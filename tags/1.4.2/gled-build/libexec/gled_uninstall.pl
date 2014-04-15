#!/usr/bin/perl

use lib "$ENV{GLEDSYS}/perllib";
use Gled_Installer;

Gled_Installer::parse_opts();

my ($d, @files) = @ARGV;

if ($Gled_Installer::HELP or not defined $d or $#files < 0)
{
  print "usage: gled_uninstall [options] <destination_dir> <files>\n" .
    Gled_Installer::option_string();
  exit 1;
}

unless (-d $d)
{
  print STDERR "Destination directory '$d' does not exist.\n";
  exit 1;
}

Gled_Installer::uninstall($d, @files);
