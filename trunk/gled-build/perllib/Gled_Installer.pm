# $Id$

# Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

package Gled_Installer;

use Carp;

########################################################################

$RECURSE = 0;
$SYMLINK = 0;
$DRYRUN  = 0;
$HELP    = 0;

########################################################################
sub parse_opts
########################################################################
{
  if (grep(/^--help$/, @ARGV))
  {
    $HELP = 1;
    return;
  }
  while ($ARGV[0] =~ m/^-(\w+)$/)
  {
    $_ = $1;
    $RECURSE = 1 if /r/;
    $SYMLINK = 1 if /s/;
    $DRYRUN  = 1 if /n/;
    $HELP    = 1 if /h/;

    shift @ARGV;
  }
}

########################################################################
sub option_string
########################################################################
{
return <<"FNORD";
Options:
  -r    recurse [$RECURSE]
  -s    symlink instead of rsync [$SYMLINK]
  -n    dry-run - echo commands insted of executing them [$DRYRUN]
  -h    help
FNORD
}

########################################################################

########################################################################
sub filter_hidden
########################################################################
{
  # This one filters-out hidden files.
  return grep { $_ !~ m/^\./ } @_;
}

########################################################################
sub find_files_and_dirs($\@\@)
########################################################################
{
  my ($path, $files, $subdirs) = @_;

  unless (opendir D, $path)
  {
    print "Gled_Installer::find_files_and_dirs $path not found ... skipping\n";
    return 0;
  }
  my @dirlist = readdir D; closedir D;

  @$files = grep (-f "$path/$_", @dirlist);
  @$files = filter_hidden(@$files);

  @$subdirs = grep (-d "$path/$_", @dirlist);
  @$subdirs = filter_hidden(@$subdirs);

  return ($#$files >= 0 or $#$subdirs >= 0);
}


########################################################################
# Low-level install functions
########################################################################

########################################################################
sub install
########################################################################
{
  my ($dest, $source, @files) = @_;

  @files = map("$source/$_", @files);

  unless (-d $dest)
  {
    system("mkdir -p $dest") and croak "mkdir of $dest failed";
  }

  if ($SYMLINK)
  {
    my @args = ("ln", "-sf", @files, $dest);
    if ($DRYRUN)
    {
      print join(" ", @args), "\n";
    }
    else
    {
      system(@args) and croak "symlink to $dest failed";
    }
  }
  else
  {
    my @args = ("rsync", "-a", @files, $dest);
    if ($DRYRUN)
    {
      print join(" ", @args), "\n";
    }
    else
    {
      system(@args) and croak "rsync to $dest failed";
    }
  }
}

########################################################################
sub uninstall
########################################################################
{
  my ($dest, @files) = @_;

  @files = map ("$dest/$_", @files);

  my @args = ("rm", "-f", @files);
  if ($DRYRUN)
  {
    print join(" ", @args), "\n";
  }
  else
  {
    system(@args) and croak "rm from $dest failed";
  }
}


########################################################################
# install of whole directories
########################################################################

########################################################################
sub install_dirs
########################################################################
{
  my $dest_dir   = shift;
  my $source_dir = shift;
  my @dirs       = @_;

  # For each dir in @dirs, installs all files found in $source_dir
  # to $dest_dir.
  # If recurse is true, the directories found in source directories
  # are installed recursively.

  for $d (@dirs)
  {
    my (@files, @subdirs);

    if (find_files_and_dirs("$source_dir/$d", @files, @subdirs))
    {
      if ($#files >= 0)
      {
        install("$dest_dir/$d", "$source_dir/$d", @files);
      }
      if ($RECURSE and $#subdirs >= 0)
      {
        install_dirs("$dest_dir/$d", "$source_dir/$d", @subdirs);
      }
    }
  }
}

########################################################################
sub uninstall_dirs
########################################################################
{
  my $dest_dir   = shift;
  my $source_dir = shift;
  my @dirs       = @_;

  # For each dir in @dirs, uninstalls all files found in $source_dir
  # from $dest_dir.
  # If recurse is true, the directories found in source directories
  # are uninstalled recursively.
  # No check is done if the directories remain empty afterwards -
  # they are always left there.

  for $d (@dirs)
  {
    my (@files, @subdirs);

    if (find_files_and_dirs("$source_dir/$d", @files, @subdirs))
    {
      if ($#files >= 0)
      {
        uninstall("$dest_dir/$d", @files);
      }
      if ($RECURSE and $#subdirs >= 0)
      {
        uninstall_dirs("$dest_dir/$d", "$source_dir/$d", @subdirs);
      }
    }
  }
}


########################################################################
# install of files
########################################################################

########################################################################
sub install_files
########################################################################
{
  my $dest_dir   = shift;
  my $source_dir = shift;

  # Installs all files in $source_dir to $dest_dir.
  # If recurse is true, the directories found in $source_dir
  # are installed recursively.

  my (@files, @subdirs);

  if (find_files_and_dirs("$source_dir", @files, @subdirs))
  {
    if ($#files >= 0)
    {
      install($dest_dir, $source_dir, @files);
    }
    if ($RECURSE and $#subdirs >= 0)
    {
      for $d (@subdirs)
      {
        install_files("$dest_dir/$d", "$source_dir/$d");
      }
    }
  }
}

########################################################################
sub uninstall_files
########################################################################
{
  my $dest_dir   = shift;
  my $source_dir = shift;

  # Uninstalls all files in $source_dir to $dest_dir.
  # If recurse is true, the directories found in $source_dir
  # are uninstalled recursively.

  my (@files, @subdirs);

  if (find_files_and_dirs("$source_dir", @files, @subdirs))
  {
    if ($#files >= 0)
    {
      uninstall($dest_dir, @files);
    }
    if ($RECURSE and $#subdirs >= 0)
    {
      for $d (@subdirs)
      {
        uninstall_files("$dest_dir/$d", "$source_dir/$d");
      }
    }
  }
}

########################################################################

1;
