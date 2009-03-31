#!/usr/bin/perl

# Writes gled-config shell script to STDOUT.
# Arguments: outfile install-type prefix bindir libdir datadir
# install-type - "BLOB" or "PREFIX"

($OUTFILE, $INSTALL_TYPE, $PREFIX, $BINDIR, $LIBDIR, $DATADIR) = @ARGV;

die "Output-file must be given, use '-' for stdout." unless $OUTFILE;

$INSTALL_TYPE = '<none>' unless $INSTALL_TYPE;

$PREFIX  = '<none>' unless $PREFIX;
$BINDIR  = '<none>' unless $BINDIR;
$LIBDIR  = '<none>' unless $LIBDIR;
$DATADIR = '<none>' unless $DATADIR;

if ($OUTFILE eq '-') {
  *GCOUT = *STDOUT;
} else {
  die "can't open $OUTFILE" unless open(GCOUT, ">$OUTFILE");
}

$INFILE = "$ENV{GLEDSYS}/bin/gled-config.in";

my $gconfig;
{
  local $/; undef $/;
  open GCIN, $INFILE or die "can not read $INFILE";
  $gconfig = <GCIN>;
  close GCIN;
}

$gconfig =~ s/\@install_type\@/$INSTALL_TYPE/g;

$gconfig =~ s/\@prefix\@/$PREFIX/g;
$gconfig =~ s/\@bindir\@/$BINDIR/g;
$gconfig =~ s/\@libdir\@/$LIBDIR/g;
$gconfig =~ s/\@datadir\@/$DATADIR/g;

print GCOUT $gconfig;

unless (*GCOUT eq *STDOUT)
{
  close GCOUT;
  chmod 0777 & ~umask, $OUTFILE;
}
