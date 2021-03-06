#!/usr/bin/perl

# Returns vendor, name and version of the OS distribution

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

sub check_windows
{
  $vendor = "Microsoft";
  ($name, $version) = `cmd /c ver` =~
     m/windows\s+([^[]+?)\s+\[version\s+([\d\.]+)/i;
}

sub check_mac
{
  $name   = "darwin";
  $vendor = "apple";
  ($version) = $os =~ m/([\d\.]+)$/;
}

########################################################################
# main
########################################################################

$os = shift;

if ($os =~ /linux-gnu/)
{
  check_linux();
}
elsif ($os =~ /cygwin/)
{
  check_windows();
}
elsif ($os =~ /darwin/)
{
  check_mac();
}
else
{
  die "unknown os '$os'"
}

print <<"FNORD"
distro_vendor=$vendor
distro_name=$name
distro_version=$version
FNORD
