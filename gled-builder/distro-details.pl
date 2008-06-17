#!/usr/bin/perl

########################################################################
# foos
########################################################################

sub slurp_file
{
  my $file = shift;
  open R, $file;
  my $r = <R>;
  close R;
  chomp $r;
  return $r;
}

sub check_linux
{
  if (-e "/etc/redhat-release") {
    $vendor = "redhat";

    my $r = slurp_file("/etc/redhat-release");
    if ($r =~ /fedora/i) {
      $name = "fedora";
    }
    elsif ($r =~ /scientific linux cern/i) {
      $name = "slc";
    }
    else {
      $name = "unknown";
    }
    ($version) = $r =~ m/([\d\.]+)/;
  }
  elsif (-e "/etc/debian_version") {
    $vendor = "debian";
    $name   = "unknown";
    my $r = slurp_file("/etc/debian_version");
    ($version) = $r =~ m/([\d\.]+)/;
  }
  elsif (-e "/etc/gentoo-release") {
    $vendor = "gentoo";
    $name   = "unknown";
    my $r = slurp_file("/etc/gentoo-release");
    ($version) = $r =~ m/([\d\.]+)/;
  }
  else {
    die "unknown distro";
    exit 1;
  }
}


########################################################################
# main
########################################################################

$os = shift;

if ($os =~ /linux-gnu/)
{
  check_linux();
  break;
}
else
{
  die "unknown os"
}

print <<"FNORD"
distro_vendor=$vendor
distro_name=$name
distro_version=$version
FNORD
