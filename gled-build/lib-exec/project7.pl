#!/usr/bin/perl

# $Header$
# project7 pargen ... second reincarnation with bloated karma
#                     expect improvements when perl6 is out

# Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

use lib "$ENV{GLEDSYS}/perllib";
use Gled_ConfCat_Parser;
use ParseConfig;
use GLED_Widgets;

my $cfg_parser = new ParseConfig(-defcfg=>"$ENV{GLEDSYS}/cfg/project7.rc");
$cfg_parser->parse();

die "must use -c option" unless defined $CLASSNAME;

########################################################################
# Preliminary sniff of the header file; class decl. stored in $c
########################################################################

$INFILE =~ s!^(\./)?!!;
if($INFILE eq '-' or not defined $INFILE) {
  @_=<>;
} else {
  open(INFILE,$INFILE) or die "can't open $INFILE";
  @_=<INFILE>;
  close INFILE;
}
$_=join('',@_);

# Parse-o-fy INFILE to get dir - base -
($INDIR, $INBASE) = $INFILE =~ m!(.*/)?(\w+)\.h!;

die "Class $CLASSNAME not matched ...\n" unless ($p, $c) =
  m!class \s+ $CLASSNAME \s+ :?\s* 
    ([\n\s:\w,]*?) \s*
    {(.*)} [\n\s]*; \s*
    //\s*endclass\s+$CLASSNAME
   !sx;

{ # extract parent classes; first parent should be glass
  my $pp = $p;
  $pp =~ s/virtual//go;
  $pp =~ s/public//go; $pp =~ s/protected//go;$pp =~ s/private//go;
  $pp =~ s/\s+//go;
  @PARENTS = split(/\s*,\s*/, $pp);
  $PARENT = $PARENTS[0];
}

print "Parents = (", join (",", @PARENTS), ")\n" if $DEBUG;

undef $PARENT if $CLASSNAME eq $BASECLASS;

########################################################################
# Extraction from catalog
########################################################################

# produces A) $config, $resolver hash-refs; B) $CATALOG;
Gled_ConfCat_Parser::import_build_config();
Gled_ConfCat_Parser::parse_catalog();

$LibID = $CATALOG->{LibID};
$LibSetName = $CATALOG->{LibSetName};
# ClassID != undef asserts that we have a glass
$ClassID = (exists $CATALOG->{Classes}{$CLASSNAME}{ClassID} ?
	    $CATALOG->{Classes}{$CLASSNAME}{ClassID} : 
	    undef);
$IsGlass = defined $ClassID;
$VirtualBase = $CATALOG->{Classes}{$CLASSNAME}{VirtualBase};
print "LibID $LibID, ClassID $ClassID, VirtualBase $VirtualBase\n" if $DEBUG;

########################################################################
# Set-up of globals
########################################################################

# Remapping of composite types into basic ones
%GetSetMap = (
  'TString' => { GetType=>'Text_t*',
		 GetMeth=>'.Data()',
		 SetArgs=>'const Text_t* s'
	       },
  'ZColor'  => { GetType=>'ZColor&',
		 SetMeth=>'.rgba(r,g,b,a)',
		 SetArgs=>'Float_t r,Float_t g,Float_t b,Float_t a=1'
	       }
);

# Shorthands for keys in comment field
%X_TO_KEY = ( 'X' => 'Xport', 'L' => 'Link', 'C' => 'Ctx', 'T' => 'Tags' );

# Recognized fields for Xporter
# g/s get/set, r ref, p ptr, t/T trans/tring, e provide just remote exec Set
# These not used any more ... a god, we really need a proper parser soon.
$Xport_FIELDS = "[gGsStTrRpPeE]";

# l ~ link to list
$Link_FIELDS = "[lL]";

# e ~ export
$Method_FIELDS = "[eE]";

@SimpleTypes = 
  ( "char", "Char_t", "unsigned char", "UChar_t", "short", "Short_t",
    "unsigned short", "UShort_t", "long", "Int_t", "unsigned long",
    "UInt_t", "int", "Int_t", "unsigned int", "UInt_t", "int", "Seek_t",
    "long", "Long_t", "unsigned long", "ULong_t", "int", "Seek_t", "long",
    "Long_t", "unsigned long", "ULong_t", "float", "Float_t", "double",
    "Double_t", "char", "Text_t", "unsigned char", "Bool_t", "unsigned char",
    "Byte_t", "short", "Version_t", "const char", "Option_t", "int", "Ssiz_t",
    "float", "Real_t", "bool",
    "ID_t", "LID_t", "CID_t","MID_t","TimeStamp_t", "UCIndex_t", "xxIndex_t");

########################################################################
# Subs
########################################################################

sub SlurpComments {
  # collects contiguous comment lines, stripping off the initial //
  # uses the wonderful $c global
  my $ret;
  while($c =~ m!\G[ \t]*//(.*)\n!omgc) {
    $ret .= $1;
  }
  return $ret;
}

sub hashofy_string {
  # chomps string into hashref, e.g. (output by Dumper):
  # "pepe, woofko, lojz=>7" -> { 'pepe' => 1, 'woofko' => 1, 'lojz' => '7' };

  my $l = shift;
  my @l = split(/\s*,\s*/, $l);
  my $r = {};
  for $e (@l) {
    if( $e =~ m/(.+)\s*=>\s*(.+)/ ) {
      $r->{$1} = $2;
    } else {
      $r->{$e} = 1;
    }
  }
  return $r;
}

########################################################################

sub SetArgs {
  my $t = shift;
  my $a = shift;
  if(exists $GetSetMap{$t} and exists $GetSetMap{$t}->{SetArgs}) {
    return $GetSetMap{$t}->{SetArgs};
  } else {
    return "$t $a";
  }
}

sub MunchArgs {
  # Transforms argument part of function declaration into an
  # array-ref of argument data as needed for streaming of method requests
  my $argument_string = shift;
  my @a = split(/\s*,\s*/, $argument_string);
  my $r = [];
  for $a (@a) {
    my $sa = [];

    # 0~whole type, 1~argname, 2~arg wo/ def value,
    # 3~type wo/ const, 4~type wo/ const but with & sign, if it applies
    # 5~actual text to be used as argument to the call in E_Exec (defaults
    #   to argument name itself).
    # This is getting relly cludgy here

    @$sa = $a =~ m/(.*?\S)\s+(\S+\s*(?:=\s*.+)?)$/;
    $sa->[0] =~ s/$/*/ if $sa->[1] =~ s/^\*//;
    $sa->[3] = $sa->[0]; $sa->[3] =~ s/^const\s+//;
    $sa->[4] = $sa->[3]; $sa->[3] =~ s/&//;
    $sa->[2] = $sa->[1]; $sa->[2] =~ s/=.*//;
    # Push in TString to char* translation
    # ???? Is this obsoleted by GetSetMap ????
    #if($sa->[3] eq 'TString') {
    #  $sa->[3] = 'char*'; $sa->[2] .= '.Data()';
    #}
    $sa->[5] = $sa->[2];
    push @$r, $sa;
  }
  return $r;
}

sub BeamArgs {
  # Returns code that streams function arguments

  my $bufp = shift;
  my $ar = shift;
  # This is slightly complex
  # Due to brain-damaged templates in eggs 1.1  must resolve this in some way.
  my $ret = "";
  for $r (@$ar) {
    if($r->[3] eq "Text_t*" || $r->[3] eq "char*") {
      $ret .= "  $bufp->WriteArray($r->[2], $r->[2] ? strlen($r->[2])+1 : 0);\n";
      next;
    }

    my $starp=""; my $arrp = ".";
    if($r->[3] =~ m/\*$/) { $starp="*"; $arrp="->"; }
    
    if( grep { $r->[3] =~ /^$_/ } @SimpleTypes) {
      $ret .= "  *$bufp << ${starp}$r->[2];\n";
      next;
    }

    if( exists $resolver->{GlassName2GlassSpecs}{$r->[3]} ) {
      $ret .= "  *$bufp << $r->[2]\->GetSaturnID();\n";
      next;
    }

    # Stream enums as Int_t
    if($r->[3] =~ m/_e$/) {
      $ret .= "  *$bufp << (Int_t)$r->[2];\n";
      next;
    }

    if($r->[0] eq $r->[4]) {
      $ret .= "   $r->[2]${arrp}Streamer(*$bufp);\n";
    } else {
      $ret .= "   const_cast<$r->[4]>($r->[2])${arrp}Streamer(*$bufp);\n";
    }
  }
  return $ret;
}

sub QeamArgs {
  # Returns code that un-streams arguments and assigns them to
  # given variable names

  my ($ar, $bufvar, $prefix) = @_;
  my $ret = "";
  for $r (@$ar) {
    my $pointerp = ($r->[3] =~ m/(.*)\*$/);
    my $puretype = $1;
    $ret .= "${prefix}";

    if($r->[3] eq "Text_t*" || $r->[3] eq "char*") {
      $ret .= "$r->[3] $r->[2]_raw=0; $bufvar.ReadArray($r->[2]_raw); ".
	      "auto_ptr<$puretype> $r->[2]($r->[2]_raw);\n";
      $r->[5] = "$r->[2].get()";
      next;
    }

    if( grep { $r->[3] =~ /^$_/ } @SimpleTypes) {
      if($pointerp) {
        $ret .= "auto_ptr<$puretype> $r->[2] (new $puretype); $bufvar >> *$r->[2];\n";
	$r->[5] = "$r->[2].get()";
      } else {
        $ret .= "$r->[3] $r->[2]; $bufvar >> $r->[2];\n";
      }
      next;
    }

    if( exists $resolver->{GlassName2GlassSpecs}{$r->[3]} ) {
      $ret .= "{ ID_t _x; $bufvar >> _x; $r->[2] = dynamic_cast<$r->[3]>(${SUNPTRVAR}->DemangleID(_x)); }\n";
      next;
    }

    # Stream enums as Int_t
    if($r->[3] =~ m/_e$/) {
      $ret .= "$r->[3] $r->[2]; { Int_t _e; $bufvar >> _e; $r->[2] = ($r->[3])_e; }\n";
      next;
    }

    if($pointerp) {
      $ret .= "auto_ptr<$puretype> $r->[2](new $puretype); $r->[2]\->Streamer($bufvar);\n";
      $r->[5] = "$r->[2].get()";
    } else {
      $ret .= "$r->[3] $r->[2]; $r->[2]\.Streamer($bufvar);\n";
    }
  }
  return $ret;
}

########################################################################

sub produce_tags {
  my ($list, $tag_string) = @_;
  my $ret = "";
  my @tags = split(/\s*,\s*/, $tag_string);
  for $t (@tags) {
    $ret .= "$list.push_back(\"$t\");";
  }
  return $ret;
}

########################################################################
# External handler registration
########################################################################

sub InstallHandler {
  my $pm = shift;
  my $pp = $pm; $pp =~ s!::!_!o;
  print "InstallHandler w/ $pm $pp\n" if $DEBUG;
  require "$pp.p7";
  import $pm;
  my $handler = eval("new $pm") or die "can't instantiate handler $pm";
  print $handler->desc() if $DEBUG;
  push @HANDLERS, $handler;
  return 1;
}

########################################################################
# Some helpers ... to be called via 7777
########################################################################

# Init for RnrCtrl
$RnrCtrl_ctor = "";
sub RnrCtrl {
  $RnrCtrl_ctor = shift;
  print "RnrCtrl($RnrCtrl_ctor)\n" if $DEBUG;
  return 1;
}

# Additional includes for views
@AdditionalViewIncludes = ();
sub AddViewInclude {
  $incl = shift;
  push @AdditionalViewIncludes, $incl;
  print "AddViewInclude($incl)\n" if $DEBUG;
  return 1;
}

########################################################################
# Parse loop
########################################################################

$MemberID = 1;
$MethodID = 257;

$state = 'private';

while($c !~ m!\G$!osgc) {
  #print "Enterring at ", pos $c,"\n" if $DEBUG;
  # Check for 7777 instructions
  if($c =~ m!\G\s*//\s*
             7777\s*([^\n]*?)\s*\n
            !omgcx)
  {
    print "evaling \"$1\"\n" if $DEBUG;
    eval($1) or die;
    next;
  }
  # Check for 777 widgets (Explicit)
  if($c =~ m!\G\s*//\s+
             777\s+(\w+\([^)]*\))*\n
            !omgcx)
  {
    my $view = $1;
    # Methodbase and Methodname should be defed in the constructor in .h file.
    $view =~ s/,\)/)/;
    my $control = eval("new GLED::Widgets::"."$view");
    push @Views, $control;
    print "777\t$view\n" if $DEBUG;
    next;
  }
  # Otherwise pure comments not expected/parsed ... whatever ...
  SlurpComments();
  if($c =~ m!\G\s*public:!osgc) {
    print "Going public ...\n" if $DEBUG;
    $state = 'public'; next;
  }
  if($c =~ m!\G\s*protected:!osgc) {
    print "Going protected ...\n" if $DEBUG;
    $state = 'protected'; next;
  }
  if($c =~ m!\G\s*private:!osgc) {
    print "Going private ...\n" if $DEBUG;
    $state = 'private'; next;
  }

  # search for public enums missing !!!

  ##############
  # Data members
  ##############
  if($c =~ m!\G\s*(?:mutable\s+)?
             ((?:const\s+)?[\w:]+\*?\&?)\s+  # type
             (\*?\w+)\s*;                 # varname
            !mgcx)
  {
    my $comment = SlurpComments();
    print "Member\t$1 $2 $comment\n" if $DEBUG;
    my $type = $1;
    my $varname = $2;
    $type =~ s/$/*/ if $varname =~ s/^\*//; # move * sign to type field (if needed)
    # strip beginning of varname for method base [like Get<methodbase>]
    #   strips 1st character and optional number; think h1
    my $methodbase = $varname; $methodbase =~ s/.\d?//;

    # Go for Key{Value} construts ... assert Xport exists before parsing on
    print "Trying for $varname: $comment\n" if $DEBUG;;
    if($comment =~ m!X|(?:Xport)\{.*\}!o) {
      my $member = {};
      my $exported = ($comment=~m/^\!/) ? 0 : 1;
      # Parse out instr{args} constructs
      print "  partitions: " if $DEBUG;;
      while($comment =~ m!(\w+)\s*\{([^}]*)\}!g) {
	my $key = $1;
	my $val = $2;
	$key = $X_TO_KEY{$key} if exists $X_TO_KEY{$key};
	$member->{$key} = $val;

	print "  $key:$val => " if $DEBUG;
	while($val =~ m!(\w)\[([^\]]*)\]!g) {
	  my $arg = lc($1);
	  my $hash = hashofy_string($2);
	  $member->{$key}{$arg} = $hash;
	  print "$1 -> $2 |" if $DEBUG;
	}
	print "\n"if $DEBUG;

      }
    
      my $settype = $type;
      $settype .= "&" if $member->{Xport}{s}{ref};
      my $argstr = &SetArgs($settype, lc($methodbase));

      $member->{Type} = $type;
      $member->{Methodbase} = $methodbase;
      $member->{Methodname} = "Set$methodbase";
      $member->{Varname} = $varname;
      $member->{ArgStr} = $argstr;
      $member->{Args} = &MunchArgs($argstr);
      $member->{Exported} = $exported;
      $member->{ID} = $exported ? $MemberID++ : 0;

      print "$type $methodbase $exported $member->{ID}\n" if $DEBUG;

      # Go for widget/view
      if($comment =~ m!7\s+(\w+\([^)]*\))!o) {
	my $view = $1;
	$view =~ s/\(/(Type=>'$type',Methodbase=>'$methodbase',Methodname=>"Set$methodbase",/;
	$view =~ s/,\)/)/;
	my $control = eval("new GLED::Widgets::".$view);
	die "$view can not be instantiated ..." unless defined $control;
	push @Views, $control;
	# range is honoured by Get/Set methods ... if defined
	# this is not perfect ... must invent widget w/ range/step control
	# ... could like spawn range/step ctrl on double click; ValuatorGroup?
	$member->{Range} =  $control->{-range} if exists $control->{-range};      
	print "\tView $view\n" if $DEBUG;
      }
    
      for $h (@HANDLERS) {
        $h->parse($member, $comment);
      }

      push @Members, $member;
    }

    next;
  } # end members if


  # Methods ...
  if($c =~ m!\G\s*
     (?:virtual)?\s+                 # Must handle virtuals, too
     ((?:const\s+)?[\w:]+(?:\*|&)?)?\s+ # Return value (optional const, */&)
     (\w+)                          # Name
     \(([\w\d\s,*&=\"\.]*)\)\s*     # Arguments ... w/ possible default values
     (const)?\s*                    # [const]
     (throw\([\w\d_]*\))?\s*        # [throw]
     (?:;|(?::[^{]+)?{.*?})         # (; | {inline def}); not greedy for *}*
      !osgcx)
  {
    my $type = $1; my $methodname = $2; my $args = $3; my $const = $4;
    #!! throw not assigned; for ()? ... do you get empty string?
    print "Method\t$1 $2 $3\n" if $DEBUG;
    my $comment = SlurpComments();

    my $ar = MunchArgs($args);
    if($DEBUG) {
      for $x (@$ar) {
	print "\t$x->[0]\t$x->[1]\n";
      }
    }
    if($methodname eq $CLASSNAME && not $VirtualBase) {
      # Here was constructor-glue.
    } elsif($comment =~ m!X|(?:Xport)\{$Method_FIELDS+\}!o) {
      my $member = {Type=>$type, Methodbase=>$methodname, Methodname=>$methodname, ID=>$MethodID++,
		    ArgStr=>$args, Args=>$ar};
      while($comment =~ m!(\w+)\s*\{([^}]*)\}!g) {
	my $key = $1;
	my $val = $2;
	$key = $X_TO_KEY{$key} if exists $X_TO_KEY{$key};
	$member->{$key} = $val;
	# It is pushed after check for range is done
	print "$key:$val " if $DEBUG;
      }
      push @Methods, $member;
    }
       
    if($comment =~ m!7\s+(\w+\([^)]*\))!o) {
      my $view = $1;
      # Args can contain " ... must backslash them
      $args =~ s/"/\\"/og;
      $view =~ s/\(/(Type=>"$type",Methodbase=>"$methodname",Methodname=>"$methodname",Const=>"$const",Args=>"$args",/;
      #print $view."\n";
      $view =~ s/,\)/)/;
      my $control = eval("new GLED::Widgets::"."$view") or die;
      push @Views, $control;
      print "\tView\t$view\n" if $DEBUG;
    }
    next;
  }

  # Hmmmh ... there are things p7 ignores ... eat line ... retry
  # Like static stuff, some empty lines, ClassDef() ... #include "xx.h7" ;)
  $c =~ m!\G(.*)\n!omgc;
  print "Fijou-fijou ... $1\n" if $DEBUG;
}

########################################################################
# OUT1FILE, option -outfile; .h7 file ... to be included in <Class>.h
########################################################################
gen1:
goto gen3 if($OUT1FILE eq 'skip');

if($OUT1FILE eq '-') {
  *H7 = *STDOUT;
} else {
  $OUT1FILE = "${INDIR}${INBASE}.h7" if $OUT1FILE eq "def";
  die "can't open $OUT1FILE" unless open(H7,">$OUT1FILE");
}

if($IsGlass) {
print H7 "// ID methods\n";

print H7 "static LID_t LibID() { return $LibID; }\n";
print H7 "virtual LID_t ZibID() const { return $LibID; }\n";
print H7 "static CID_t ClassID() { return $ClassID; }\n";
print H7 "virtual CID_t ZlassID() const { return $ClassID; }\n";
print H7 "static FID_t FID() { return FID_t($LibID,$ClassID); }\n";
print H7 "virtual FID_t ZID() const { return FID_t($LibID,$ClassID); }\n";
print H7 "virtual const char* ZlassName() const { return \"$CLASSNAME\"; }\n";
print H7 "virtual const char* ZibName() const { return \"$LibSetName\"; }\n";
print H7 "\n";
}

for $r (@Members) {
  # Get methods
  if( $r->{Xport} =~ m/(g|G)/ ) {
    my $const = ($1 eq 'G') ? 'const' : '';
    my ($const, $type, $val, $constret, $pre, $post);
    $const = ' const' if $1 eq 'G'; # and not($IsGlass && $LOCK_GET_METHS); 
    if(exists $GetSetMap{$r->{Type}}) {
      my $h = $GetSetMap{$r->{Type}};
      $type = "$h->{GetType}";
      $val = "$r->{Varname}$h->{GetMeth}";
      $constret = 'const ' if $1 eq 'G';
      #print H7 "const " if $const and  $h->{GetType} =~ /&$/;
      #print H7 "$h->{GetType}\tGet$r->{Methodbase}()\t$const\t{ return $r->{Varname}$h->{GetMeth}; }\n";
    } else {
      $type = "$r->{Type}";
      $val = "$r->{Varname}";
      $constret = 'const ' if $1 eq 'G' and  $h->{GetType} =~ /&|\*$/;
      #print H7 "$r->{Type}\tGet$r->{Methodbase}()\t$const\t{ return $r->{Varname}; }\n";
    }
    if($IsGlass && $LOCK_GET_METHS) {
      $pre = "mExecMutex.Lock(); ";
      $post= "mExecMutex.Unlock(); ";
    }
    print H7 "${constret}${type} Get$r->{Methodbase}()${const} ".
      "{ ${pre}${constret}${type} _ret = ${val}; ${post}return _ret; }\n";
  }

  if( $r->{Xport} =~ m/(s|S)/ ) {
    my ($pre, $setit, $post, $stamp, $ret);
    if($IsGlass) {
      $pre     .= "mExecMutex.Lock(); " if $LOCK_SET_METHS;
      if(exists $r->{Link}) {
	$pre   .= "if($r->{Varname}) $r->{Varname}->DecRefCount(this); ";      
      }
    }
    if($1 eq 'S' and $IsGlass) {
      $stamp .= "mStampReqTrans = " if $r->{Xport} =~ m/t/;
      $stamp .= "mStampReqTring = " if $r->{Xport} =~ m/T/;
      if(exists $r->{Link}) {
	$stamp .= "StampLink(LibID(), ClassID()); ";
      } else {
	$stamp .= "Stamp(LibID(), ClassID()); ";
      }
    } else {
      $stamp = '';
    }
    if($IsGlass) {
      if(exists $r->{Link}) {
	$post  .= "if($r->{Varname}) $r->{Varname}->IncRefCount(this); "      
      }
      $post    .= "mExecMutex.Unlock(); " if $LOCK_SET_METHS;
    }

    $r->{Type} =~ /(.)/; my $arg = lc $1;
    print H7 "void $r->{Methodname}($r->{ArgStr}) {";
    if(exists $r->{Range}) { # Check if range is set ... make if stuff
      my $arg = $r->{Args}[0][2]; # assume single argument
      my $rr = $r->{Range};
      print H7 "\n\tif($arg>$rr->[1] || $arg<$rr->[0]) return;\n\t";
      $ret = "\n}\n";
    } else {
      print H7 " ";
      $ret =  "}\n";
    }
    $setit .= "$r->{Varname}";
    $setit .=
      ((exists $GetSetMap{$r->{Type}} and exists $GetSetMap{$r->{Type}}->{SetMeth}) ?
      $GetSetMap{$r->{Type}}->{SetMeth} : " = $r->{Args}[0][2]") . "; ";
    print H7 "${pre}${setit}${stamp}${post}${ret}";
  }
  
  # Produce ZGlass* version with dynamic_cast for links;
  # Do it also if the Set method is hand-written (e|E specifier)
#  if($r->{Xport} =~ m/(e|E)/ && exists $r->{Link} && not($r->{Type} eq "${BASECLASS}*")) {
#    print H7<<"fnord";
#void Set$r->{Methodbase}(ZGlass* d) {
#  $r->{Type} dd=0; if(d) dd = dynamic_cast<$r->{Type}>(d);
#  if(d==0 || dd) Set$r->{Methodbase}(dd);
#}
#fnord
#  }

  if( $r->{Xport} =~ m/(r|R)/ ) {
    my $const = ($1 eq 'R') ? 'const ' : '';
    print H7 "${const}$r->{Type}& Ref$r->{Methodbase}() ${const}\{ return $r->{Varname}; }\n";
  }
  if( $r->{Xport} =~ m/(p|P)/ ) {
    my $const = ($1 eq 'P') ? 'const ' : '';
    print H7 "${const}$r->{Type}* Ptr$r->{Methodbase}() ${const}\{ return &$r->{Varname}; }\n";
  }
}
print H7 "\n";

for $h (@HANDLERS) {
  $h->spit_h7(*H7);
}

# For non-blessed classes this is the end of the ride ...
goto gen1_end unless $IsGlass;

print H7 "// Link exporter\n";

print H7 "virtual void CopyLinks(lpZGlass_t& glass_list, Bool_t lockp=true);\n";
print H7 "virtual void CopyLinkRefs(lppZGlass_t& ref_list);\n";
print H7 "virtual void CopyLinkSpecs(lLinkSpec_t& link_spec_list);\n";
print H7 "virtual Int_t RebuildLinks(ZComet* c);\n";
print H7 "\n";

print H7 "// Declarations of remote-exec methods\n";
print H7 "void ExecuteMir(ZMIR& mir);\n";

for $r (@Members) {
  next unless $r->{Xport} =~ m/s|S|e|E/o && $r->{Exported};
  print H7 "ZMIR* S_$r->{Methodname}($r->{ArgStr});\n";
  print H7 "static MID_t Mid_$r->{Methodname}() { return $r->{ID}; }\n";
}

for $r (@Methods) {
  print H7 "ZMIR* S_$r->{Methodbase}($r->{ArgStr});\n";
  print H7 "static MID_t Mid_$r->{Methodbase}() { return $r->{ID}; }\n";
}
print H7 "\n";

print H7 "static void _gled_catalog_init();\n\n";

gen1_end:
close H7 unless *H7==*STDOUT;

########################################################################
# OUT3FILE, option -3file; .c7 file ... to be included in <Class>.cxx
########################################################################
gen3:
goto gen5 if($OUT3FILE eq 'skip');

if($OUT3FILE eq '-') {
  *C7 = *STDOUT;
} else {
  $OUT3FILE = "${INDIR}${INBASE}.c7" if $OUT3FILE eq "def";
  die "can't open $OUT3FILE" unless open(C7,">$OUT3FILE");
}

for $h (@HANDLERS) {
  $h->spit_c7(*C7);
}

goto gen3_end if not $IsGlass;

#################################
### Forest following and building
#################################

print C7 "#include <Ephra/Saturn.h>\n";
print C7 "#include <Gled/GledNS.h>\n";
print C7 "#include <Stones/ZMIR.h>\n";
print C7 "#include <Stones/ZComet.h>\n";
print C7 "#include <memory>\n\n";

unless($CLASSNAME eq $BASECLASS) {
  print C7 "#define PARENT_GLASS ${PARENT}\n\n";
}

# LinkList
print C7 "void\n${CLASSNAME}::CopyLinks(lpZGlass_t& glass_list, Bool_t lockp) {\n";
print C7 "  if(lockp) mExecMutex.Lock();\n";
unless($CLASSNAME eq $BASECLASS) {
  print C7 "  ${PARENT}::CopyLinks(glass_list, false);\n"
}
for $r (@Members) {
  next unless exists $r->{Link};
  my ($pure_type) = $r->{Type} =~ m/(.*?)\*/;
  die "Link must be a pointer" if $pure_type eq $r->{Type};
  print C7 "  glass_list.push_back($r->{Varname});\n";
}
print C7 "  if(lockp) mExecMutex.Unlock();\n";
print C7 "}\n\n";

# LinkRefList
print C7 "void\n${CLASSNAME}::CopyLinkRefs(lppZGlass_t& ref_list) {\n";
unless($CLASSNAME eq $BASECLASS) {
  print C7 "  ${PARENT}::CopyLinkRefs(ref_list);\n"
}
for $r (@Members) {
  next unless exists $r->{Link};
  my ($pure_type) = $r->{Type} =~ m/(.*?)\*/;
  die "Link must be a pointer" if $pure_type eq $r->{Type};
  my $glass_var = ($r->{Type} eq "${BASECLASS}*") ?
    "&$r->{Varname}" : "(ZGlass**)(&$r->{Varname})";
  print C7 "  ref_list.push_back($glass_var);\n";
}
print C7 "}\n\n";

# LinkSpec
print C7 "void\n${CLASSNAME}::CopyLinkSpecs(lLinkSpec_t& link_spec_list) {\n";
unless($CLASSNAME eq $BASECLASS) {
  print C7 "  ${PARENT}::CopyLinkSpecs(link_spec_list);\n"
}
for $r (@Members) {
  next unless exists $r->{Link};
  my ($pure_type) = $r->{Type} =~ m/(.*?)\*/;
  die "Link must be a pointer" if $pure_type eq $r->{Type};
  my $glass_var = ($r->{Type} eq "${BASECLASS}*") ?
    "&$r->{Varname}" : "(ZGlass**)(&$r->{Varname})";
  print C7 "  link_spec_list.push_back( LinkSpec(\"$CLASSNAME\", \"$r->{Methodbase}\") );\n";
}
print C7 "}\n\n";

# RebuildLinks
unless($CATALOG->{Classes}{$CLASSNAME}{C7_DoNot_Gen}{RebuildLinks}) {
  print C7 "Int_t\n${CLASSNAME}::RebuildLinks(ZComet* c) {\n";
  print C7 "  Int_t ret".(defined $PARENT ? "=${PARENT}::RebuildLinks(c)" : "=0").";\n";
  # Here should rebuild [...|r...] marked; NodeLists are recoverd from above
  for $r (@Members) {
    next unless exists $r->{Link};
    print C7 "  if($r->{Varname} != 0) {\n";
    print C7 "    $r->{Varname} = dynamic_cast<$r->{Type}>(c->FindID((UInt_t)$r->{Varname}));\n";

    print C7 "  }\n";
  }
  print C7 "  return ret;\n}\n\n";
}

#######################
### Remote-exec methods
#######################

for $r (@Members) {
  next unless $r->{Xport} =~ m/s|S|E|e/o && $r->{Exported};
  if(exists $r->{Link}) {
    print C7 <<"fnordlink";
ZMIR* ${CLASSNAME}::S_$r->{Methodname}($r->{ArgStr}) {
  ZMIR* _mir = new ZMIR(mSaturnID, ($r->{Args}[0][1] ? $r->{Args}[0][1]\->GetSaturnID() : 0));
  _mir->SetLCM_Ids($LibID, $ClassID, $r->{ID});
  return _mir;
}

fnordlink
  } else {
  my $args1 = join(", ", map( { "$_->[0] $_->[2]" } @{$r->{Args}}));
  my $args2 = join(", ", map( { $_->[2] } @{$r->{Args}}));
  print C7 <<"fnord";
ZMIR* ${CLASSNAME}::S_$r->{Methodname}($args1) {
  ZMIR* _mir = new ZMIR(mSaturnID);
  _mir->SetLCM_Ids($LibID, $ClassID, $r->{ID});
fnord
  print C7 BeamArgs("_mir", $r->{Args});
  print C7 <<"fnord";
  return _mir;
}

fnord
  } # end if Link
}

### Others/Explicit/Exported/Executable Methods
for $r (@Methods) {
  my $args1 = join(", ", map( { "$_->[0] $_->[2]" } @{$r->{Args}}));
  my $args2 = join(", ", map( { $_->[2] } @{$r->{Args}}));
  my $c = exists $r->{Ctx} ? substr($r->{Ctx},0,1) : 0;
  my $C = $#{$r->{Args}};
  print C7 "ZMIR*\n${CLASSNAME}::S_$r->{Methodbase}($args1) {\n";
  if(exists $r->{Ctx}) {
    print C7 "  ZMIR* _mir = new ZMIR(mSaturnID" . ($c>0 ? ", " : " ") .
    join(", ", map( { "($_->[2] ? $_->[2]\->GetSaturnID() : 0)" } @{$r->{Args}}[0 .. $c-1])) .
    ");\n";
  } else {
    print C7 "  ZMIR* _mir = new ZMIR(mSaturnID);\n";
  }
  print C7 "  _mir->SetLCM_Ids($LibID, $ClassID, $r->{ID});\n";
  my @aa = @{$r->{Args}}[$c .. $C];
  print C7 BeamArgs("_mir", \@aa);
  print C7 <<"fnord";
  return _mir;
}

fnord
}

##############
### ExecuteMir
##############

if($IsGlass) {
  print C7<<"fnord";
void ${CLASSNAME}::ExecuteMir(ZMIR& mir) {
  static string _eh("${CLASSNAME}::ExecuteMir ");
  static string _bad_ctx("ctx argument of wrong type");
  switch(mir.Mid) {
fnord

  # Set stuff
  for $r (@Members) {
    next unless $r->{Xport} =~ m/s|S|e|E/o && $r->{Exported};
    if(exists $r->{Link}) {

      print C7 << "fnordlink";
  case $r->{ID}: {
    $r->{Type} _beta = dynamic_cast<$r->{Type}>(mir.Beta);
    if(mir.Beta != 0 && _beta == 0)
      throw(_eh + "[$r->{Methodname}] " + _bad_ctx);
    $r->{Methodname}(_beta);
    break;
  }
fnordlink

    } else {

      print C7 "  case $r->{ID}: {\n";
      print C7 QeamArgs($r->{Args}, "mir", "    ");
      my @ca = map { $_->[5] } (@{$r->{Args}});
      print C7 "    $r->{Methodname}(". join(", ", @ca) .");\n    break;\n  }\n";

    } # end if Link
  }

  # Others
  for $r (@Methods) {
    print C7 "  case $r->{ID}: {\n";
    my $c = exists $r->{Ctx} ? substr($r->{Ctx},0,1) : 0;
    my $C = $#{$r->{Args}};
    if(exists $r->{Ctx}) {
      my $cc = 0; my @names = ("Beta", "Gamma", "_context_too_long_");
      while($cc < $c) {
	$ar = $r->{Args}[$cc];
	print C7 << "fnord";
    $ar->[0] $ar->[2] = dynamic_cast<$ar->[0]>(mMir->$names[$cc]);
    if($ar->[2] == 0 && mMir->$names[$cc] != 0)
      throw(_eh + "[$r->{Methodbase}] " + _bad_ctx + ":$ar->[2] [$names[$cc]]");
fnord
	++$cc;
      }
      my @aa = @{$r->{Args}}[$c .. $C];
      print C7 QeamArgs(\@aa, "mir", "    ");
    } else {
      print C7 QeamArgs($r->{Args}, "mir", "    ");
    }
    my @ca = map { $_->[5] } (@{$r->{Args}});
    print C7 "    $r->{Methodbase}(". join(", ", @ca) .");";
    print C7 "\n    break;\n  }\n";
  }
  # end
  print C7 " default: { }\n";
  print C7 " } // end switch\n}\n\n";
}

#####################
### Catalog generator
#####################

print C7 <<"fnord";
namespace { GledNS::ClassInfo* _ci=0; }

void ${CLASSNAME}::_gled_catalog_init() {
  using namespace GledNS;

  if(_ci) return;
  _ci = new ClassInfo("${CLASSNAME}", FID_t($LibID, $ClassID));
  _ci->fParentName = "$PARENT";
  _ci->fParentCI = 0;
fnord

#####################
### Member/MethodInfo
#####################

for $r (@Members) {
  if($r->{Xport} =~ m/s|S|e|E/o && $r->{Exported}) {
    print C7 "  {\n    MethodInfo* mip = new MethodInfo(\"$r->{Methodname}\", $r->{ID});\n";
    if(exists $r->{Link}) {
      print C7 "    mip->fContextArgs.push_back(\"$r->{Args}[0][0] $r->{Args}[0][1]\");\n";
    } else {
      print C7 "    mip->fArgs.push_back(\"$r->{Args}[0][0] $r->{Args}[0][1]\");\n";
    }
    if(exists $r->{Tags}) {
      print C7 "    " .  produce_tags("mip->fTags", $r->{Tags}) . "\n";
    }
    print C7 "    mip->fClassInfo = _ci;\n";
    print C7 "    _ci->fMethodList.push_back(mip);\n";
    print C7 "    _ci->fMethodHash[$r->{ID}] = mip;\n\n";

    if(exists $r->{Link}) {
      print C7 "    LinkMemberInfo* lmip = new LinkMemberInfo(\"$r->{Methodbase}\");\n";
      print C7 "    lmip->fType = \"$r->{Args}[0][0]\";\n";
      print C7 "    lmip->fSetMethod = mip;\n";
      print C7 "    _ci->fLinkMemberList.push_back(lmip);\n";      
    } else {
      print C7 "    DataMemberInfo* dmip = new DataMemberInfo(\"$r->{Methodbase}\");\n";
      print C7 "    dmip->fType = \"$r->{Args}[0][0]\";\n";
      print C7 "    dmip->fSetMethod = mip;\n";
      print C7 "    _ci->fDataMemberList.push_back(dmip);\n";      
    }
    print C7 "  }\n";
  }
}

for $r (@Methods) {
  if($r->{Xport} =~ m/s|S|e|E/o) {
    print C7 "  {\n    MethodInfo* mip = new MethodInfo(\"$r->{Methodname}\", $r->{ID});\n";
    my $c = exists $r->{Ctx} ? substr($r->{Ctx}, 0, 1) : 0;
    my $C = $#{$r->{Args}};
    for($i=0; $i<=$C; ++$i) {
      if($i < $c) {
	print C7 "    mip->fContextArgs.push_back(\"$r->{Args}[$i][0] $r->{Args}[$i][1]\");\n";
      } else {
	print C7 "    mip->fArgs.push_back(\"$r->{Args}[$i][0] $r->{Args}[$i][1]\");\n";
      }
    }
    if(exists $r->{Tags}) {
      print C7 "    " .  produce_tags("mip->fTags", $r->{Tags}) . "\n";
    }
    print C7 "    mip->fClassInfo = _ci;\n";
    print C7 "    _ci->fMethodList.push_back(mip);\n";
    print C7 "    _ci->fMethodHash[$r->{ID}] = mip;\n\n";
    print C7 "  }\n";
  }
}
print C7 "  GledNS::BootstrapClass(_ci);\n}\n\n";

gen3_end:
close C7 unless *C7==*STDOUT;

########################################################################
# OUT5FILE, option -5file; In fact .h file for class <Class>View
########################################################################
gen5:
goto gen7 if($OUT5FILE eq 'skip');

if($OUT5FILE eq '-') {
  *H = *STDOUT;
} else {
  $OUT5FILE = "Views/${CLASSNAME}View.h" if $OUT5FILE eq "def";
  die "can't open $OUT5FILE" unless open(H,">$OUT5FILE");
}

print H <<"fnord";
// Header file for class ${CLASSNAME}View
// It has been generated automagickally from $INFILE.

#ifndef ${LibSetName}_${CLASSNAME}View_H
#define ${LibSetName}_${CLASSNAME}View_H

#include <Eye/MTW_View.h>
#include <Eye/MTW_SubView.h>
#include <$INFILE>
fnord

{
  my %done = ();
  my @incs = map {$_->{Include} } @Views;
  for $r (@incs, @AdditionalViewIncludes) {
    if(not exists $done{$r}) {
      print H "#include <$r>\n";
      $done{$r} = 1;
    }
  }
}

print H "\nclass ${CLASSNAME}View : public MTW_SubView {\n";

for $r (@Views) {
  print H $r->make_header_ccbu();
}

print H <<"fnord";
  ${CLASSNAME}* mIdol;
public:
  ${CLASSNAME}View(GledNS::ClassInfo* ci, MTW_View* v, ${CLASSNAME}* i) :
    MTW_SubView(ci, v), mIdol(i) {}

  static MTW_SubView* Construct(GledNS::ClassInfo* ci, MTW_View* v, ZGlass* g);
  static void _gled_catalog_init();
};

#endif
fnord
close H unless *H==*STDOUT;

########################################################################
# OUT7FILE, option -7file; In fact .cxx file for class <Class>View
########################################################################
gen7:
goto heaven if($OUT7FILE eq 'skip');

if($OUT7FILE eq '-') {
  *C = *STDOUT;
} else {
  $OUT7FILE = "Views/${CLASSNAME}View.cxx" if $OUT7FILE eq "def";
  die "can't open $OUT7FILE" unless open(C,">$OUT7FILE");
}

print C <<"fnord";
// Source file for class ${CLASSNAME}View
// It has been generated automagickally from $INFILE.

#include "${CLASSNAME}View.h"
#include <Eye/Eye.h>
#include <memory>\n
fnord

#####################################
### Creator, Callback and Update foos
#####################################

for $r (@Views) {
  print C $r->make_widget();
  print C $r->make_cxx_cb();
  print C $r->make_weed_update();

  print C "// -----\n\n";
}

# SubView Creator
print C <<"fnord";
MTW_SubView*
${CLASSNAME}View::Construct(GledNS::ClassInfo* ci, MTW_View* v, ZGlass* g) {
  ${CLASSNAME}* tg = dynamic_cast<${CLASSNAME}*>(g);
  if(!tg) return 0;
  return new ${CLASSNAME}View(ci, v, tg);
}

fnord

#####################
### Catalog generator
#####################

print C <<"fnord";
namespace { GledViewNS::ClassInfo* _ci=0; }

void ${CLASSNAME}View::_gled_catalog_init() {
  using namespace GledViewNS;
  if(_ci) return;
  _ci = new ClassInfo;
  _ci->fooSVCreator = &Construct;
  _ci->fRendererGlass = "$CATALOG->{Classes}{$CLASSNAME}{RnrClass}";
  _ci->fDefRnrCtrl = RnrCtrl(${RnrCtrl_ctor});
  _ci->fRendererCI = 0;
  GledNS::ClassInfo* master = GledNS::FindClassInfo(FID_t($LibID, $ClassID));
  master->fViewPart = _ci;
fnord

### No special data for Methods & DataMembers

################
# LinkMemberInfo
################

for $r (@Members) {
  if(exists $r->{Link}) {
    print C <<"fnord";
  {
    LinkMemberInfo* lmip = new LinkMemberInfo;
    lmip->fDefRnrBits = RnrBits($r->{RnrBits});
    GledNS::LinkMemberInfo* master_lmip = master->FindLinkMemberInfo("$r->{Methodbase}", false);
    master_lmip->fViewPart = lmip;
  }
fnord
  }
}

################
# Views -> Weeds
################

for $r (@Views) {
  print C <<"fnord";
  {
    WeedInfo* wi = new WeedInfo("$r->{Methodbase}");
    wi->fWidth = $r->{-width};
    wi->fHeight= $r->{-height};
    wi->bLabel = $r->{LabelP};
    wi->bLabelInside = $r->{LabelInsideP};
    wi->bCanResize = $r->{CanResizeP};
    wi->bJoinNext = $r->{-join};
    wi->fooWCreator  = $r->{Methodbase}_Creator_s;
    wi->fooWCallback = (WeedCallback_foo) $r->{Methodbase}_Callback_s;
    wi->fooWUpdate   = (WeedUpdate_foo)   $r->{Methodbase}_Update_s;
    _ci->fWeedList.push_back(wi);
  }
fnord
}


print C "  GledViewNS::BootstrapClassInfo(_ci);\n}\n\n";

close C unless *C==*STDOUT;

########################################################################
heaven:
exit(0);
