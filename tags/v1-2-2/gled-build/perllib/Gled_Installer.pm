# $Header$

# Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

package Gled_Installer;

use Carp;

sub filter_files {
  # This one filters-out hidden files.
  my @out = grep { $_ !~ m/^\./ } @_;
  return @out;
}

sub install_dirs {
  my $dest_dir   = shift;
  my $source_dir = shift;
  my @dirs = @_;

  # For each $dir in @dirs, installs all files found in $source/$dir
  # to $dest/$dir

  for $d (@dirs) {
    unless( opendir D, "$source_dir/$d" ) {
      # print "Gled_Installer::install_dirs $source_dir/$d not found ... skipping\n";
      next;
    }
    my @files = readdir D;
    closedir D;
    @files = grep (-f "$source_dir/$d/$_", @files);
    @files = filter_files(@files);
    @files = map  ("$source_dir/$d/$_", @files);

    if($#files < 0) {
      # print "Gled_Installer::install_dirs $source_dir/$d empty ... skipping\n";
      next;
    }

    unless(-d "$dest_dir/$d") {
      system("mkdir $dest_dir/$d")
	and croak "creation of $dest_dir/$d failed";
    }
    system("rsync", "-a", @files, "$dest_dir/$d")
      and croak "copy to $dest_dir/$d failed";
  }
}

sub uninstall_dirs {
  my $dest_dir   = shift;
  my $source_dir = shift;
  my @dirs = @_;

  # For each $dir in @dirs, uninstalls all files found in $source/$dir
  # from $dest/$dir

  for $d (@dirs) {
    unless( opendir D, "$source_dir/$d" ) {
      # print "Gled_Installer::uninstall_dirs $source_dir/$d not found ... skipping\n";
      next;
    }
    my @files = readdir D;
    closedir D;
    @files = grep (-f "$source_dir/$d/$_", @files);
    @files = filter_files(@files);

    if($#files < 0) {
      # print "Gled_Installer::uninstall_dirs $source_dir/$d empty ... skipping\n";
      next;
    }

    @files = map  ("$dest_dir/$d/$_", @files);
    system("rm", "-f", @files);
  }
}

1;
