#!/afs/cern.ch/user/a/aliprod/.alien/software/Installation/alien/bin/perl -w

use AliEn::UI::Catalogue::LCM::Computer;
use AliEn::Database::TaskQueue;
use Data::Dumper;

my $options =
{
 'user'       => $ENV{ALIEN_USER},    
 'debug'      => 0,
 'exec'       => "",
 'token'      => "",
 'password'   => "",
 'silent'     => 1,
 'role'       => "",
 'AuthMethod' => "",
 'domain'     => "",
 'organisation'=>"",
};

$CMDFIFO = shift;
$OUTFIFO = shift;

if($OUTFIFO) {
  open OUT, ">$OUTFIFO";
} else {
  *OUT = *STDOUT;
}

select(OUT);    $| = 1;
select(STDOUT); $| = 1;

$ali = new AliEn::UI::Catalogue::LCM::Computer($options);
$cat = $ali->{CATALOG};
$ce  = $ali->{QUEUE};
{
  local *STDOUT;  open STDOUT, ">/dev/null";
  $ce->f_queue("info"); # inits low-level dbs: TASK, PRIORITY, ADMIN
  close STDOUT;
}

print OUT "OK\n";

if($CMDFIFO) {
  open CMD,  "$CMDFIFO";
} else {
  *CMD = *STDIN;
}

########################################################################
########################################################################


sub show_tables {
  my $db = shift;
  my $dbcls = ref $db;
  print '#' x 72 . "\nSHOW TABLES for $dbcls\n" . '-' x 72 . "\n";
  my $res = $db->_queryDB("show tables");
  # print Dumper($res);
  for $r (@$res) {
    for $k (keys %$r) {
      printf "%-24s %s\n", $k,  $r->{$k};
    }
  }
}

########################################################################

sub describe {
  my ($db, $tbl) = @_;
  my $dbcls = ref $db;
  print '#' x 72 . "\nDESCRIBE $tbl (db=$dbcls)\n" . '=' x 72 . "\n";
  printf "%-16s %-24s %-8s <%s,%s>\n",
    "Type", "Field", "Default", "Null", "Key";
  print '-' x 72 . "\n";
  my $res = $db->_queryDB("describe $tbl");
  # print Dumper($res);
  for $r (@$res) {
    printf "%-16s %-24s %-8s <%s,%s>\n",
      $r->{Type}, $r->{Field}, "'$r->{Default}'", $r->{Null}, $r->{Key};
  }
}

sub dump_sites_sql {
  my $db = shift;
  my $qry = "select * from SITES;";
  my $res = $db->_queryDB($qry);

  local $, = "\n", $\ = "\n";

  if(ref($res) eq "ARRAY") {
    my $num = $#{@$res} + 1;
    print OUT "OK", $num;
    for $r (@$res) {
      # my $name = "$r->{siteName}\@$r->{location}<$r->{domain}>";
      # printf "%-36s [%3s,%3s] (%s,%s)\n",
      #  $name, $r->{siteId}, $r->{masterHostId}, $r->{latitude}, $r->{longitude};
      print OUT $r->{siteName}, $r->{location},     $r->{domain},
                $r->{siteId},   $r->{masterHostId}, 
                $r->{latitude}, $r->{longitude};
    }
  } else {
    print OUT "ERR", $qry;
  }
}

sub xx {
  my ($a, $v) = @_;
  my $r = $a->get_value($v);
  $r = "" unless defined $r;
  $r =~ s/\s*$//o;
  return $r;
}

sub extract_site_values {
  my $a = shift;
  return ( xx($a,'ou'), xx($a,'location'), xx($a,'domain'),
	   xx($a,'latitude'), xx($a,'longitude') );
  
}

sub dump_sites {
  my $part = shift;

  my $ldap = $ali->{CONFIG}->GetLDAPDN();

  if($part eq "") { # Retrieve all sites
    my $msg =  $ldap->search
      ( base   => "ou=Sites,$ali->{CONFIG}{LDAPDN}",
	filter => "(objectClass=AliEnSite)" );

    my $num = $msg->count;
    # print "Got $total replies:\n";

    local $, = "\n", $\ = "\n";
    print OUT "OK", $num;
    for($i=0; $i<$num; ++$i) {
      print OUT extract_site_values( $msg->entry($i) );
    }
  } else {          # Retrive a partition only.
    my $pq = $ldap->search
      ( base   => "name=$part,ou=Partitions,$ali->{CONFIG}{LDAPDN}",
	filter => "(objectClass=AliEnPartition)" );
    my $npq = $pq->count;
    if($npq != 1) {
      print OUT "ERR", "Search for partition '$part' yielded $npq results.";
      goto end;
    }
    my @sites = $pq->entry(0)->get_value('CEname');
    @sites = map { split /::/; $_[1]; } @sites;

    local $, = "\n", $\ = "\n";
    print OUT "OK", $#sites + 1;
    my $site_i = 1;
    for $s (@sites) {
      my $sq = $ldap->search
      ( base   => "ou=$s,ou=Sites,$ali->{CONFIG}{LDAPDN}",
	filter => "(objectClass=AliEnSite)" );
      my $nsq = $sq->count;
      if($nsq != 1) {
	print "Strange ... got $nsq results for site $s.";
	unless($nsq) { 
	  print OUT "FOO-$site_i", "Nowhereland", "foo.bar", "89", "0";
	}
      }
      print OUT extract_site_values( $sq->entry(0) );
      ++$site_i;
    }
  }
 end:
  $ldap->unbind;
}

########################################################################

sub find_ex {
  my ($path, $wcard) = @_;

  print "now running findEx($path, $wcard)\n";
  $res = $cat->findEx("$path", "$wcard");
  print "  returned $res, $#{$res}\n";

  local $, = "\n", $\ = "\n";
  print OUT "OK", $#{$res} + 1;
  for $r (@$res) {
    my $site = (split /::/, $r->{pfns}[0]{se})[1];
    print OUT $r->{lfn}, $site;
  }
}

########################################################################

sub dump_hosts {
  my $db = shift;
  print '#' x 72 . "\nAliEn Hosts\n" . '-' x 72 . "\n";
  my $res = $db->_queryDB("select * from HOSTS");
  # print Dumper($res);
  for $r (@$res) {
    my $name = "$r->{hostName}:$r->{hostPort}";
    my $d = gmtime $r->{date};
    my $state = "$r->{status}@\"$d\"";
    printf "%-36s [%3s,%3s] %s\n",
      $name, $r->{siteId}, $r->{hostId}, $state;
  }
}

sub raw_dump_table {
  my ($db, $tbl) = @_;
  my $dbcls = ref $db;
  print '#' x 72 . "\nRAW DUMP of SELECT * from $tbl (db=$dbcls)\n" . '-' x 72 . "\n";
  my $res = $db->_queryDB("select * from $tbl");
  print Dumper($res);
}

########################################################################
########################################################################

#describe($ce->{TASK_DB}, "SITES");
#dump_sites($ce->{TASK_DB});

while($_ = <CMD>) {
  print "How! Got $_";
  next if /^\s*$/;
  chomp;

  if (/^dump_sites$/) {

    my $part = <CMD>; chomp $part;
    dump_sites($part);

  } elsif (/^find_ex$/) {

    my $path  = <CMD>; chomp $path;
    my $wcard = <CMD>; chomp $wcard;
    find_ex($path, $wcard);

  } else {
    print OUT "ERR\nUnknown command.\n";
  }
}

end:
print "Closing stuff.\n";
close CMD unless *CMD == *STDIN;
close OUT unless *OUT == *STDOUT;

$ali->close();

print "Closed all stuff.\n";
