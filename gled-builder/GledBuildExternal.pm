########################################################################
# Common configuration and functions
########################################################################

use Carp;
use GledBuildConf;

########################################################################
# Common variables
########################################################################

$VER_RE    = '\d+\.\d+(?:\.\d+)?(?:-[\w\d]+)??';
$SOURCE_RE = '(?:src|source|gled)';
$TAR_RE    = '(?:tar\.gz|tgz)';

@FILES_IN_CACHE_DIR = slurp_dir("$CACHE_DIR");


########################################################################
# System functions
########################################################################

sub exec_or_die {
  my $cmd = shift;
  if($DRYRUN) {
    print "EXEC: $cmd\n";
  } else {
    my $ret = `$cmd`;
    croak "$cmd died" if $?;
    return $ret
  }
}

sub system_or_die {
  my $cmd = shift;
  if($DRYRUN) {
    print "SYST: $cmd\n";
  } else {
    system "$cmd" and die "$cmd died";
  }    
}

sub cd {
  my $dir = shift;
  if($DRYRUN) {
    print "CD: $dir\n";
    return 1;
  } else {
    return chdir $dir;
  }
}

sub slurp_dir {
  my $dir = shift;
  opendir D, $dir;
  my @files = readdir D;
  closedir D;
  @files = grep { -f "$dir/$_" and !/^\./ } @files; # grep out .files, . and ..
  return @files;
}


########################################################################
# Package functions
########################################################################

sub setup_package {
  # Find source-tarball for package in cache-dir or attempt to
  # download it from cache-url.
  #
  # Returns package file, version and top-level directory name within
  # the tar in global variables $srctarfile, $version and $tardir.
  #
  # If this can not be achieved it dies with an appropriate error.

  my $pkg = shift;

  my $download_tried = 0;
 entry:
  my @files = grep { /^${pkg}-${VER_RE}(?:-${SOURCE_RE})?\.${TAR_RE}$/ }
                   @FILES_IN_CACHE_DIR;

  my $file;

  if ($#files == 0) {
    $file = $files[0];
  }
  elsif ($#files < 0) {
    if ($download_tried) {
      die "Download of package $pkg seemed to succeed but the file still not found."
    }
    print STDERR "Package $pkg not found in local cache ... attempting download.\n";
    $download_tried = 1;
    download_package($pkg);
    goto entry;
  }
  elsif ($#files > 0) {
    die "Several matching packeges found in cache:\n  ",
        join("\n  ", @files),
        "\nAborting.";
  }

  $file =~ m/$pkg-(${VER_RE})(?:-${SOURCE_RE})?\.${TAR_RE}$/;

  $srctarfile = $file;
  $version    = $1;
  $tardir     = deduce_tardir_name("$CACHE_DIR/$file");

  print <<"FNORD";
HOMEPAGE   := $homepage

SRCTARFILE := $srctarfile
VERSION    := $version
TARDIR     := $tardir
FNORD
}

sub download_package {
  my $pkg = shift;

  if (not -e $CACHE_DIR) {
    system_or_die("mkdir -p $CACHE_DIR");
  }
  system_or_die("cd $CACHE_DIR; rsync -av $CACHE_URL/$pkg-*.tar.gz . 1>&2");
  @FILES_IN_CACHE_DIR = slurp_dir("$CACHE_DIR");
}

sub deduce_tardir_name {
  my $tar = shift;
  open F, "tar tzf $tar|" or die "failed listing tar file $tar";
  my $dir = <F>; chomp $dir;
  close F;
  $dir =~ s!/.*$!!o;
  return $dir;
}


########################################################################
# Target generation functions
########################################################################

@required_tgts = ( 'configure', 'build', 'install' );
%done_tgts     = ();
%default_cmds  = (
  'configure' => "./configure --prefix=$PREFIX",
  'build'     => "make",
  'install'   => "make install"
);

sub target {
  # Register commands to be run for target.
  # The commands can be like this:
  # echo command
  # echo a very long command that \\
  #    is split already here
  # echo and yet another one
  # echo `pwd`
  
  my $tgt = shift;
  my $cmd = shift;

  chomp $cmd;
  $cmd =~ s/(?<!\\)\n(?=.)/; \\\n/g;

  print "\n";
  print <<"FNORD";
define $tgt-cmds
$cmd
endef
FNORD

  ++$done_tgts{$tgt};
}

sub use_defaults_for_remaining_targets {
  for $tgt (@required_tgts) {
    target($tgt, $default_cmds{$tgt}) unless exists  $done_tgts{$tgt};
  }
}

########################################################################
# End
########################################################################

1;
