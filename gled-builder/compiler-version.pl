#!/usr/bin/perl

$cc = shift;

if ($cc =~ /gcc/) {
  $ver = `$cc -dumpversion`;
  chomp $ver;
}
elsif ($cc =~ /cl/) {
  ($ver) = `$cc /help 2>&1 | head -n 1` =~ m/version\s+([\d\.]+)/i;
}
else {
  die "unknown compiler '$cc'";
}

split /\./, $ver;

print <<"FNORD"
CC_VERSION=$ver
CC_MAJOR=$_[0]
CC_MINOR=$_[1]
FNORD
