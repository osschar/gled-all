#!/usr/bin/perl

use lib ".";
use GledBuildExternal;

if ($ARGV[0] eq '-h' or $ARGV[0] eq '--help')
{
  print <<"FNORD";
Usage: $0 [--rebuild]
Refreshes list of external packages available on the server and downloads
the new versions of packages that were already in the cache.
If --rebuild is specified, the affected packages are also rebuilt.
FNORD
  exit 0;
}

my @updated_pkgs = update_cache();

my $make_cmd = "make -C external " . join(" ", @updated_pkgs);

if (@updated_pkgs)
{
  if ($ARGV[0] eq '--rebuild')
  {
    system_or_die($make_cmd);
  }
  else
  {
    print "Some packages have been updated. Run the following command to rebuild them:\n";
    print "$make_cmd\n";
  }
}
else
{
  print "No updates necessary.\n";
}
