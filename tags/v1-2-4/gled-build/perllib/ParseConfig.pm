# Copyright (C) 1999-2004, Matevz Tadel.
# Released under The Artistic License, 1997.

package ParseConfig;

# Parses a configuration file defining command line options and
# default values for global variables.

use Getopt::Long;

# Default values are evaled ... so be careful
# Legal to return \@ or \% ... but read Getopt::Long for what it means and
# how such cases are treated.

sub new {
  my $proto = shift;
  my $class = ref($proto) || $proto;
  my $S = {@_};
  bless($S, $class);

  # -defcfg, -cfgbase, -useenv, -verbose, -hash
  # pass defcfg as string or arr-ref ... it *WILL* become aref
  if(defined $S->{-defcfg}) {
    $S->{-defcfg} = [ $S->{-defcfg} ] unless ref $S->{-defcfg} eq "ARRAY";
  } else {
    $S->{-defcfg} = [];
  }
  my $cfgbase;
  if(defined $S->{-cfgbase}) {
    $cfgbase = $S->{-cfgbase};
  } else {
    $0 =~ m!([^/]+?)(?:.pl)?$!;
    $cfgbase = $1;
  }
  $S->{ProgName} = $cfgbase;
  push @{$S->{-defcfg}}, "$ENV{PWD}/${cfgbase}.rc",
                         "$ENV{PWD}/.${cfgbase}.rc",
	                 "$ENV{HOME}/cfg/${cfgbase}.rc",
                         "$ENV{HOME}/.${cfgbase}.rc";
  $S->{PostFoos} = [];

  return $S;
}

sub add_post_foo {
  my ($S, $foo) = @_;
  push @{$S->{PostFoos}}, $foo;
}

sub parse {
  # This shit should be split in several routines ... 
  my $S = shift;
  my $ar = shift;

  if(defined $ar) {
    @ARGV = @$ar;
    print "Substituting \@ARGV ...\n "if $S->{-verbose};
  }

  if($ARGV[0] eq "-cfg") {
    shift @ARGV; $S->{Config} = shift @ARGV;
  } else {
    for $c (@{$S->{-defcfg}}) {
      if(-r $c) {
        $S->{Config} = $c;
	last;
      }
    }
  }

  $S->{CmdlOpts} = [];
  $S->{Vars} = [];

  # first let's find the file
  die "ParseConfig::parse: config file '$S->{ProgName}' not found\n"
    unless -r $S->{Config};
  print "Using config $S->{Config} ...\n" if $S->{-verbose};
  print "Using environment overrides of defaults ...\n"
    if $S->{-useenv} and $S->{-verbose};

  open CFG, $S->{Config};
  while(<CFG>) {
    next if /^#/ || /^\s/;
    chomp;
    my ($conf, $type, $context, $var, $def) = split(' ',$_,5);
    my ($varref, $symref);
    # Env overrides?
    if($S->{-useenv} && defined $ENV{$var}) {
      $def = $ENV{$var};
    }
    # Set default value
    if($S->{-hash}) {
      if($context eq 'main' or $context eq ".") {
	$S->{-hash}{$var} = eval $def;
	$varref = ref ($S->{-hash}{$var}) ?
	  $S->{-hash}{$var} : \$S->{-hash}{$var};
      } else {
	$S->{-hash}{$context}{$var} = eval $def;
	$varref = ref ($S->{-hash}{$context}{$var}) ?
	  $S->{-hash}{$context}{$var} : \$S->{-hash}{$context}{$var};
      }
      $symref = 0; # not used for hashes
    } else {
      $context = "main" if $context eq ".";
      $symref = "${context}::$var";
      ${$symref} = eval $def;
      $varref = ref ${$symref} ? ${$symref} : \${$symref};
    }
    # Store some details
    push @{$S->{Vars}}, [$varref, $symref, $context, $var, $def];
    # voodoo for Getopt
    if($type ne 'x' and $type ne 'exclude') {
        $type='' if $type eq 'b' or $type eq 'bool';
        push @{$S->{CmdlOpts}}, "$conf$type", $varref;
    }
  }
  GetOptions(@{$S->{CmdlOpts}});
  for $f (@{$S->{PostFoos}}) {
    if($S->{-hash}) {
      &$f($S->{-hash});
    } else {
      &$f();
    }
  }
}

sub parse_string {
  # Splits string argument into an array, then calls parse
  my ($S, $str) = @_;
  my @a = split(' ', $str);
  # rejoin what was unjustfully split (' and "). what a pain ... do it stupidly
  # also strips them off after a match is found
  my ($n, $np, $inm) = (0, -1, 0);
  while($n<=$#a) {
    if($inm and $a[$n]=~m/$inm$/) {
      my $subst = join(' ', @a[$np, $n]);
      substr $subst,0,1,''; substr $subst,-1,1,'';
      splice @a, $np, $n-$np+1, $subst;
      $n = $np+1; $np = -1; $inm = 0;
      redo;
    }
    elsif(not $inm and $a[$n]=~m/^([\'\"])/) {
      $np = $n; $inm = $1;
    }
    $n++;
  }
  $S->parse(\@a);
}

##########################################################################
# Stoopid stuff that mostly belongs somewhere else

sub pook_href {
  # asserts keys are in hash ... otherwise assign defaults
  my $h = shift;
  die "pook_href: this not a hashref" unless ref $h eq "HASH";
  my $d = {@_};
  for $k (keys %$d) {
    if($d->{$k} eq "<required>") {
      die "required key $k missing from given hash" unless exists $h->{$k};
      next;
    }
    $h->{$k} = $d->{$k} unless exists $h->{$k};
  }
}

1;
