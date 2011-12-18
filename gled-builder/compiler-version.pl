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

@v = split /\./, $ver;

print <<"FNORD"
CC_VERSION=$ver
CC_MAJOR=$v[0]
CC_MINOR=$v[1]
FNORD
