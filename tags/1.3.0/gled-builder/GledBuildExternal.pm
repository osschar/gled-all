########################################################################
# Common configuration and functions
########################################################################

use Carp;
use Data::Dumper;
use GledBuildConf;


########################################################################
# Common variables
########################################################################

$VER_RE    = '\d+\.\d+(?:\.\d+)?(?:-[\w\d]+)??';
$SOURCE_RE = '(?:src|source|gled)';
$TAR_RE    = '(?:tar\.gz|tgz)';


########################################################################
# System functions
########################################################################

sub exec_or_die
{
  my $cmd = shift;
  if ($DRYRUN) {
    print "EXEC: $cmd\n";
  } else {
    my $ret = `$cmd`;
    croak "$cmd died" if $?;
    return $ret
  }
}

sub system_or_die
{
  my $cmd = shift;
  if ($DRYRUN) {
    print "SYST: $cmd\n";
  } else {
    system "$cmd" and die "$cmd died";
  }    
}

sub cd
{
  my $dir = shift;
  if ($DRYRUN) {
    print "CD: $dir\n";
    return 1;
  } else {
    return chdir $dir;
  }
}


########################################################################
# Package functions
########################################################################

sub setup_package
{
  # Find source-tarball for package in cache-dir or attempt to
  # download it from cache-url.
  #
  # Returns package file, version and top-level directory name within
  # the tar in global variables $srctarfile, $version and $tardir.
  #
  # If this can not be achieved it dies with an appropriate error.

  my $pkg = shift;
  my $file;

  my %cachelist  = list_files($CACHE_DIR);

  if (defined $cachelist{$pkg})
  {
    $file = $cachelist{$pkg}{'file'};
  }
  else
  {
    my %serverlist = read_or_create_serverlist();

    if (defined $serverlist{$pkg})
    {
      $file =$serverlist{$pkg}{'file'};
      download_file($file);
    }
    else
    {
      print STDERR <<"FNORD";
Tarball for package $pkg not found in cache nor in the current serverlist.
Maybe "update_cache.pl" in gled-builder/ could help.
FNORD
      exit 1;
    }
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

sub download_file
{
  my $file = shift;

  if (not -e $CACHE_DIR) {
    system_or_die("mkdir -p $CACHE_DIR");
  }
  system_or_die("cd $CACHE_DIR; rsync -av $CACHE_URL/$file . 1>&2");
}

sub deduce_tardir_name
{
  my $tar = shift;
  open F, "tar tzf $tar|" or die "failed listing tar file $tar";
  my $dir = <F>; chomp $dir;
  close F;
  $dir =~ s!/.*$!!o;
  return $dir;
}


########################################################################
# Filemap and severlist handling functions
########################################################################

sub list_files
{
  # Given a directory path, lists the files existing there (via rsync)
  # and returns them as a hash.

  my ($dir) = @_;

  # --list-only not supported on older rsyncs.
  my @ls = `rsync $dir/*-*.* 2>/dev/null`;

  my %map;

  for $l (@ls)
  {
    my @fs = split(/\s+/, $l);
    my $datime = "$fs[2]-$fs[3]";
    my $file   = $fs[4];
    my ($package) = $file =~ m/^([\w-]+)-${VER_RE}(?:-${SOURCE_RE})?\.${TAR_RE}$/;

    if (not defined $map{$package} or $map{$package}{'datime'} lt $datime)
    {
      $map{$package} = { 'datime' => $datime, 'file' => $file };
    }
  }

  return %map;
}

sub write_serverlist
{
  my ($map) = @_;

  if (not -e $CACHE_DIR) {
    system_or_die("mkdir -p $CACHE_DIR");
  }

  $Data::Dumper::Indent = 1;
  $Data::Dumper::Purity = 1;

  open C, ">$CACHE_DIR/serverlist" or die "Can't open serverlist for writing.";

  print C Dumper($map);

  close C;
}

sub read_serverlist
{
  do "$CACHE_DIR/serverlist" or die "Eval of serverlist failed.";

  return %$VAR1;
}

sub read_or_create_serverlist
{
  if (-e "$CACHE_DIR/serverlist")
  {
    return read_serverlist();
  }
  else
  {
    my %files_on_server = list_files($CACHE_URL);
    write_serverlist(\%files_on_server);
    return %files_on_server;
  }
}

sub update_cache
{
  # Updates the 'serverlist' file.
  # Compares server directory with local cache and if a newer version
  # of an existin package is found does the following:
  #  - removes local copy;
  #  - downloads the latest tarball;
  #  - calls 'make distclean' in affected package.
  # Returns a list of updated packages.

  my %files_on_server = list_files($CACHE_URL);
  write_serverlist(\%files_on_server);

  my %files_in_cache  = list_files($CACHE_DIR);

  my @updated_pkgs;

  for $k (sort keys %files_in_cache)
  {
    if ($files_in_cache{$k}{'datime'} lt $files_on_server{$k}{'datime'})
    {
      unlink "$CACHE_DIR/$files_in_cache{$k}{'file'}";
      exec_or_die("rsync $CACHE_URL/$files_on_server{$k}{'file'} $CACHE_DIR");
      exec_or_die("make -C external/$k distclean");
      push @updated_pkgs, $k;
    }
  }
  return @updated_pkgs;
}


########################################################################
# Target generation functions
########################################################################

@required_tgts = ( 'configure', 'build', 'install' );
%done_tgts     = ();

sub target
{
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

sub use_defaults_for_remaining_targets
{
  my %default_cmds  = (
    'configure' => "./configure --prefix=$PREFIX",
    'build'     => $parallel ? "make ${MAKE_J_OPT}" : "make",
    'install'   => "make install"
  );

  for $tgt (@required_tgts)
  {
    target($tgt, $default_cmds{$tgt}) unless exists  $done_tgts{$tgt};
  }
}

########################################################################
# End
########################################################################

1;
