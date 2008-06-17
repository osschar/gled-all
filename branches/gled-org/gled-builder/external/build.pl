#!/usr/bin/perl

# Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

use Carp;
use lib ".";
use ParseConfig;

$def_cfg = "build.rc";

my $cfg_parser = new ParseConfig(-defcfg=>$def_cfg, -useenv=>1);
$cfg_parser->parse();

if($HELP) {
  print "usage: build.pl <options>\n";
  print "Options:\n";
  print `cat $def_cfg`;
  exit 0;
}

if($ECHO_CONFIG) {
  print "GLED_VERSION=$GLED_VERSION\n";
  print "GLED_CVS_TAG=$GLED_CVS_TAG\n";
  do "$CONFIG" or croak "Error reading config file '$CONFIG'";
  print "GLED_LIBSETS='" . join(" ", @$LibSets) . "'\n";
  exit 0;
}

die "Buildname must be set" unless $BUILDNAME;

$TOPDIR    = "$ENV{PWD}";
$TOPBINDIR = "$TOPDIR/binary-$BUILDNAME";
$TOPSRCDIR = "$TOPDIR/source";

$INSTDIR  = "$TOPDIR/inst.tmp/$BUILDNAME";
$BUILDDIR = "$TOPDIR/build.tmp/$BUILDNAME";

$EXT_DISTDIR  = "$TOPDIR/required-software";
$EXT_SRCDIR   = "$EXT_DISTDIR/source";
$EXT_BINDIR   = "$EXT_DISTDIR/binary-$BUILDNAME";
$EXT_PREBUILTDIR = "$EXT_DISTDIR/externally_built";

$LIBSET_DIR    = "$TOPDIR/libsets";
$LIBSET_SRCDIR = "$LIBSET_DIR";
$LIBSET_BINDIR = "$LIBSET_DIR/binary-$BUILDNAME";

$ENVDIR = "$LIBSET_DIR";

$DEMODIR = "$TOPDIR/demos";

$VER_RE    = '\d+\.\d+(?:\.\d+)?(?:-[\w\d]+)??';
$SOURCE_RE = '(?:src|source|gled)';
$TAR_RE    = '(?:tar\.gz|tgz)';

do "$CONFIG" or croak "Error reading config file '$CONFIG'";
die "No source directory of required-software '$EXT_SRCDIR'" unless -d $EXT_SRCDIR;
die "No libset source directory '$LIBSET_SRCDIR'" unless -d $LIBSET_SRCDIR;
die "No environment directory '$ENVDIR'" unless -d "$ENVDIR";

if($EXTERNAL) {
  @EXTERNAL_PACKAGES = split(/:/, $EXTERNAL);
  $SKIP_EXTERNAL = 0;
  $SKIP_LIBSET   = 1;
  $SKIP_META     = 1;
} else {
  @EXTERNAL_PACKAGES = @$Packages;
}

exec_or_die("rm -rf $BUILDDIR $INSTDIR") unless $NOREMOVE;
exec_or_die("mkdir -p $BUILDDIR $INSTDIR");

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

sub deduce_package_stuff {
  my $pkg = shift;
  
  my @files = grep { /^${pkg}-${VER_RE}(?:-$SOURCE_RE)?\.${TAR_RE}/ }
                   @FILES_IN_SRC;

  if($#files != 0) {
    croak "source tarball for $pkg not found" if $#files < 0;
    croak "several tarballs found for $pkg:\n\t" . join("\n\t", @files) . "\n";
  }
  my $file = $files[0];
  $file =~ m/$pkg-($VER_RE)(?:-$SOURCE_RE)?\.$TAR_RE/;
  return ($file, $1);
}

sub deduce_dir_name {
  my $tar = shift;
  open F, "tar tzf $tar|" or die "failed listing tar file $tar";
  my $dir = <F>; chomp $dir;
  close F;
  $dir =~ s!/.*$!!o;
  return $dir;
}

########################################################################


%smart_default =
  (
   'configure' => './configure --prefix=%i/%p',
   'build'     => 'make',
   'install'   => 'make install'
  );

sub smart_exec {
  my ($pkg, $x, $what) = @_;

  my @lines;
  if(not exists($x->{$what})){ push @lines, $smart_default{$what}; }
  elsif(not ref($x->{$what})) { push @lines, $x->{$what}; }
  elsif(ref($x->{$what}) eq "ARRAY") { @lines = @{$x->{$what}}; }
  else { die "'$what' of $pkg should be null, scalar or array ref";}

  for $l (@lines) {
    $l =~ s/%i/$INSTDIR/g;
    $l =~ s/%p/$pkg/g;
    $l =~ s/%v/$x->{specs}{version}/g;
    $l =~ s/%s/$SYSNAME/g;
    system_or_die($l);
    
  }
  $x->{specs}{$what} = join("\n", @lines);
}

########################################################################

print '#' x 80 ."\n";
print "# Building external software\n";
print '#' x 80 ."\n\n";

print '#' x 10 . " Scanning packages ...\n";

@FILES_IN_SRC = slurp_dir("$EXT_SRCDIR");
printf "%-12s %-28s %-12s %s\n", "Package", "Tar-file", "Version", "Directory";
print '-' x 80;
for $p (@EXTERNAL_PACKAGES) {
  die "Package $p not in Packalog" unless defined $Packalog->{$p};
  my ($f, $v) = deduce_package_stuff($p);
  my $d = deduce_dir_name("$EXT_SRCDIR/$f");
  printf "%-12s %-28s %-12s %s\n", $p, $f, $v, $d;
  $Packalog->{$p}{'specs'} =
    { 'srctarfile' => "$EXT_SRCDIR/$f",
      'bintarfile' => "$EXT_BINDIR/$p-$v.tar",
      'bintarfile_gz' => "$EXT_BINDIR/$p-$v.tar.gz",
      'version' => $v,
      'dirname' => $d,
    };
}

if($SKIP_EXTERNAL) {
  print "\n" . '#' x 10 . " Skipping build of external software ...\n\n";
  goto external_build_end;
}

exec_or_die("rm -rf $EXT_BINDIR");
exec_or_die("mkdir -p $EXT_BINDIR");

print '#' x 10 . " Attacking packages ...\n";

for $p (@EXTERNAL_PACKAGES) {
  my $x = $Packalog->{$p};

  print '#' x 8 . " Unpacking $p ...\n";

  cd("$BUILDDIR");
  exec_or_die("tar xzf $x->{specs}{srctarfile}");
  cd("$x->{specs}{dirname}") or die "couldn't cd into build dir of $p";
  if(defined $x->{buildsubdir}) {
    print "cding to $x->{buildsubdir}\n";
    print `ls`;
    cd("$x->{buildsubdir}") or die "couldn't cd into build-subdir of $p";
    print `ls`;
  }

  print '#' x 8 . " Configuring $p ...\n";
  smart_exec($p, $x, 'configure');

  print '#' x 8 . " Building $p ...\n";
  smart_exec($p, $x, 'build');

  print '#' x 8 . " Installing $p ...\n";
  smart_exec($p, $x, 'install');

  if(exists $x->{postinstall}) {
    print '#' x 8 . " Executing Post-Install script for $p ...\n";
    smart_exec($p, $x, 'postinstall');
  }
}


print '#' x 10 . " Preparing binary tarballs ...\n";

unless($DRYRUN) {
  open  TOPBLA,">$EXT_BINDIR/README_auto" or
    die "can't create build README";
  print TOPBLA "Author   : $AUTHOR\n";
  print TOPBLA "Buildname: $BUILDNAME\n";
  print TOPBLA "uname -a : " . `uname -a`;
  print TOPBLA "date     : " . `date`;
  print TOPBLA "Package listing:\n";
}

for $p (@EXTERNAL_PACKAGES) {
  my $x = $Packalog->{$p};

  print '#' x 8 . " Munching $p ...\n";
  cd("$INSTDIR/$p");
  unless($DRYRUN) {
    open  BLA, ">README_${p}_auto" or die "can't create README for $p"; 
    print BLA "$p-$x->{specs}{version}, auto-built as an external package for GLED.\n\n";
    print BLA $x->{comment} . "\nHomepage: " . $x->{homepage} . "\n";
    print BLA "\nBuild details:\n";
    print BLA "Builder  : $AUTHOR\n";
    print BLA "Buildname: $BUILDNAME\n";
    print BLA "uname -a : " . `uname -a`;
    print BLA "date     : " . `date`;
    print BLA "\n";
    print BLA "configure  : ", $x->{specs}{configure}, "\n";
    print BLA "build      : ", $x->{specs}{build}, "\n";
    print BLA "install    : ", $x->{specs}{install}, "\n";
    print BLA "postinstall: ", $x->{specs}{postinstall}, "\n";
    close BLA;
    
    print TOPBLA "  $p-$x->{specs}{version}: $x->{specs}{configure}\n";
  }

  system_or_die("tar cf $x->{specs}{bintarfile} *");

  unless($KEEP_EXTERNAL) {
    print '#' x 8 . " Removing build & install directories for $p ...\n";
    cd("$INSTDIR");  exec_or_die("rm -rf $p");
    cd("$BUILDDIR"); exec_or_die("rm -rf $x->{specs}{dirname}");
  }
}
unless($DRYRUN) {
  close TOPBLA;
}

print '#' x 10 . " Compressing binary tarballs ...\n";
cd("$EXT_BINDIR");
system_or_die("gzip -f -9 *.tar");

external_build_end:


########################################################################
### Libset building
########################################################################

print '#' x 80 ."\n";
print "# Building Libsets\n";
print '#' x 80 ."\n\n";

if($SKIP_LIBSET) {
  print '#' x 10 . " Skipping build of libsets ...\n\n";
  goto libset_build_end;
}

print '#' x 10 . " Unpacking gled environment ...\n";

$GLED_DIR  = "$BUILDDIR/gled";
exec_or_die("rm -rf $GLED_DIR $LIBSET_BINDIR");
exec_or_die("mkdir -p $GLED_DIR $LIBSET_BINDIR");
cd($GLED_DIR);
exec_or_die("tar xzf $ENVDIR/gled.tar.gz");
$GLED_DIR .= "/" . deduce_dir_name("$ENVDIR/gled.tar.gz");

print '#' x 10 . " Unpacking libset sources ...\n";

cd($GLED_DIR);
for $ls (@$LibSets) {
  print '#' x 4 . " Unpacking $ls [$LIBSET_SRCDIR/${ls}-src.tar.gz]\n";
  exec_or_die("tar xzf $LIBSET_SRCDIR/${ls}-src.tar.gz");
}

print '#' x 10 . " Unpacking external binaries ...\n";

exec_or_die("mkdir -p external");
cd("external");
for $p (@EXTERNAL_PACKAGES) {
  print '#' x 4 . " Unpacking $p [$Packalog->{$p}{specs}{bintarfile_gz}]\n";
  exec_or_die("tar xzf $Packalog->{$p}{specs}{bintarfile_gz}");
}
cd($GLED_DIR);

print '#' x 10 . " Creating build script ...\n";
unless($DRYRUN) {
  open BS, ">build_script";
  print BS<<"fnord";
. binenv.sh
./configure --buildversion=$GLED_VERSION --ext=$GLED_DIR/external --libsets='<auto>' || exit 1
make -j2 || exit 1
fnord

  for $ls (@$LibSets) {
    print BS "BIN_DIST_TAR=${LIBSET_BINDIR}/${ls}.tar make -C $ls bin_dist || exit 1\n";
  }
  close BS;
  chmod 0755, "build_script";
}

print '#' x 10 . " Running build script ...\n";

system_or_die("./build_script");
cd($TOPDIR);

unless($KEEP_LIBSETS) {
  print '#' x 8 . " Removing Gled build directory ...\n";
  exec_or_die("rm -rf $GLED_DIR");
}

print '#' x 10 . " Compressing binary libsets ...\n";

exec_or_die("gzip -f -9 $LIBSET_BINDIR/*.tar");

libset_build_end:

########################################################################
### Packing MetaPackages
########################################################################

print '#' x 80 ."\n";
print "# Preparing MetaPackages\n";
print '#' x 80 ."\n\n";

if($SKIP_META) {
  print '#' x 10 . " Skipping build of meta packages ...\n\n";
  goto meta_build_end;
}

exec_or_die("rm -rf $TOPBINDIR");
exec_or_die("mkdir -p $TOPBINDIR");

for $mp (@$MetaPackages) {
  print '#' x 10 . " Meta packing $mp ...\n";

  my $pd = $Metalog->{$mp}; # PackageData

  if($pd->{type} eq 'source' and not $META_PACK_SRCS) {
    print '#' x 10 . " Skipping source package $mp ...\n";
    next;
  }

  cd($BUILDDIR);
  exec_or_die("rm -rf $mp");
  exec_or_die("mkdir $mp");
  cd($mp);

  my $dir_name;
  if($pd->{env} =~ m/^<dir:(\w+)>$/) {
    $dir_name = $1;
    exec_or_die("mkdir $dir_name");
  } else {
    die "Environment tar $pd->{env} does not exist" unless -f $pd->{env};
    $dir_name = deduce_dir_name($pd->{env});
    system_or_die("tar xzf $pd->{env}");
  }
  cd($dir_name);

  print '#' x 6 . " Unpacking externals ...\n";
  unless($pd->{shallow_externals}) {
    exec_or_die("mkdir -p external");
    cd("external");
  }
  for $p (@{$pd->{packages}}) {
    system_or_die("tar xzf $Packalog->{$p}{specs}{bintarfile_gz}");
  }
  cd("..") unless $pd->{shallow_externals};

  print '#' x 6 . " Unpacking libsets ...\n";
  for $ls (@{$pd->{libsets}}) {
    system_or_die("tar xzf $LIBSET_BINDIR/${ls}.tar.gz");
  }

  print '#' x 6 . " Unpacking freetars ...\n";
  for $f (@{$pd->{freetars}}) {
    system_or_die("tar xzf $f");
  }

  print '#' x 6 . " Creating & Compressing the tar ...\n";
  cd("..");
  my $tar_dirs = "$dir_name";
  if(defined $pd->{symlink}) {
    system_or_die("ln -s $dir_name $pd->{symlink}");
    $tar_dirs .= " $pd->{symlink}";
  }
  system_or_die("tar cf $pd->{dirname}/${mp}.tar $tar_dirs");
  system_or_die("gzip -f -9 $pd->{dirname}/${mp}.tar");

  unless($KEEP_META) { 
    cd("..");
    exec_or_die("rm -rf $mp");
  }
  print "\n";
}

print "#" x 4 . " Creating README files ...\n";
unless($DRYRUN) {
  open  TOPBLA, ">$TOPDIR/README_${BUILDNAME}_auto" or
    die "can't create meta build README";
  print TOPBLA "Author   : $AUTHOR\n";
  print TOPBLA "Buildname: $BUILDNAME\n";
  print TOPBLA "date     : " . `date`;
  print TOPBLA "Package listing:\n\n";

  for $mp (@$MetaPackages) {
    my $pd = $Metalog->{$mp};
    my $loc = $pd->{dirname}; $loc =~ s/$TOPDIR//;
    my $env = $pd->{env};
    $env =~ s/$TOPDIR// unless($env =~ m/^</);
    print TOPBLA "MetaPackage $mp:\n",
    "  Location:    ", $loc, "\n",
    "  Environment: ", $env, "\n",
    "  External:    ", join(", ", @{$pd->{packages}}), "\n",
    "  Libsets:     ", join(", ", @{$pd->{libsets}}), "\n",
    "  Freetars:    ", join(", ", map { s/$TOPDIR//; $_ } @{$pd->{freetars}}), "\n\n"; 
  }

  close TOPBLA;
}

meta_build_end:
