#!/usr/bin/perl

$cc = shift;

if ($cc =~ /gcc/) {
  $ver = `$cc -dumpversion`;
  chomp $ver;
}
else {
  die "unknown compiler";
}

split /\./, $ver;

print <<"FNORD"
CC_VERSION=$ver
CC_MAJOR=$_[0]
CC_MINOR=$_[1]
FNORD
