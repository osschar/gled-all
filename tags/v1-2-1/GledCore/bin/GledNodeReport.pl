#!/usr/bin/perl

die "usage: $0 <commands>\n" unless $#ARGV >= 0;

while ($command = shift) {
  
  if ($command eq "cpuinfo") {
    open F, "/proc/cpuinfo";
    while ($_=<F>) {
      if (/^processor/) {
	++$cpu_num; next;
      }
      if (/^model name\s*:\s*(.*)/) {
	$cpu_mod = $1; next;
      }
      if (/^cpu MHz\s*:\s*([\d\.]+)/) {
	$cpu_frq += $1; next;
      }
    }
    close F;
    $cpu_frq /= $cpu_num; $cpu_frq = int $cpu_frq;
    print "$cpu_mod\n$cpu_frq $cpu_num\n";

    next;
  }

  if ($command eq "meminfo") {
    open F, "/proc/meminfo";
    $_=<F>;
    $_=<F>; split; print int($_[1]/1024/1024) . " ";
    $_=<F>; split; print int($_[1]/1024/1024) . "\n";
    close F;
    next;
  }

  if ($command eq "cpureport") {
    open F, "/proc/stat";
    while ($_=<F>) {
      if (/^cpu\s+([\d ]+)/) {
	my $js = $1;
	@jiffies = split /\s+/, $js; next;
      }
    }
    close F;
    print join(" ", @jiffies) . "\n";
    open F, "/proc/loadavg";
    $_ = <F>; split;
    print join(" ", @_[0 .. 2]) . "\n";
    close F;
    next;
  }

  if ($command eq "memfree") {
    open F, "/proc/meminfo";
    $_=<F>;
    $_=<F>; split; print int(($_[3] + $_[6])/1024/1024) . " ";
    $_=<F>; split; print int($_[3]/1024/1024) . "\n";
    close F;
    next;
  }
}
