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
%X_TO_KEY = ( 'X' => 'Xport', 'L' => 'Link', 'C' => 'Ctx' );

# Recognized fields for Xporter
# g/s get/set, r ref, p ptr, t/T trans/tring, e provide just remote exec Set
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
    "ID_t", "LID_t", "CID_t","TimeStamp_t", "UCIndex_t", "xxIndex_t");

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
    if($sa->[3] eq 'TString') {
      $sa->[3] = 'char*'; $sa->[2] .= '.Data()';
    }
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

  my $ar = shift; my $prefix = shift;
  my $ret = "";
  for $r (@$ar) {
    my $pointerp = ($r->[3] =~ m/(.*)\*$/);
    my $puretype = $1;
    $ret .= "${prefix}";

    if($r->[3] eq "Text_t*" || $r->[3] eq "char*") {
      $ret .= "$r->[3] $r->[2]_raw=0; buf->ReadArray($r->[2]_raw); ".
	      "auto_ptr<$puretype> $r->[2]($r->[2]_raw);\n";
      $r->[5] = "$r->[2].get()";
      next;
    }

    if( grep { $r->[3] =~ /^$_/ } @SimpleTypes) {
      if($pointerp) {
        $ret .= "auto_ptr<$puretype> $r->[2] (new $puretype); *buf >> *$r->[2];\n";
	$r->[5] = "$r->[2].get()";
      } else {
        $ret .= "$r->[3] $r->[2]; *buf >> $r->[2];\n";
      }
      next;
    }

    if( exists $resolver->{GlassName2GlassSpecs}{$r->[3]} ) {
      $ret .= "{ ID_t _x; *buf >> _x; $r->[2] = dynamic_cast<$r->[3]>(${SUNPTRVAR}->DemangleID(_x)); }\n";
      next;
    }

    if($pointerp) {
      $ret .= "auto_ptr<$puretype> $r->[2](new $puretype); $r->[2]\->Streamer(*buf);\n";
      $r->[5] = "$r->[2].get()";
    } else {
      $ret .= "$r->[3] $r->[2]; $r->[2]\.Streamer(*buf);\n";
    }
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
    # Type and Methodbase should be defed in GLED::Widgets::xx
    # So far nothing is checked and ZTransCtrl weed created ... how lame !!!!!!
    $view =~ s/,\)/)/;
    my $control = eval("new GLED::Widgets::"."$view");
    push @ExpViews, $control;
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
             ((?:const\s+)?[\w:]+\*?)\s+  # type
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
    if($comment =~ m!X|(?:Xport)\{$Xport_FIELDS+\}!o) {
      my $argstr = &SetArgs($type, lc($methodbase));
      my $member = { Type=>$type, Methodbase=>$methodbase, Varname=>$varname,
		     ArgStr=>$argstr, Args=>&MunchArgs($argstr) };
      print "  partitions: " if $DEBUG;;
      while($comment =~ m!(\w+)\s*\{([^}]*)\}!g) {
	my $key = $1;
	my $val = $2;
	$key = $X_TO_KEY{$key} if exists $X_TO_KEY{$key};
	$member->{$key} = $val;
	# It is pushed after check for range is done
	print "$key:$val " if $DEBUG;
      }
      print "\n" if $DEBUG;

      # Go for widget/view
      if($comment =~ m!7\s+(\w+\([^)]*\))!o) {
	my $view = $1;
	$view =~ s/\(/(Type=>'$type',Methodbase=>'$methodbase',/;
	$view =~ s/,\)/)/;
	my $control = eval("new GLED::Widgets::".$view);
	die "$view can not be instantiated ..." unless defined $control;
	push @VarViews, $control;
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
      push @Constructors, {Type=>$type, Methodbase=>$methodname, ArgStr=>$args, Args=>$ar};
    } elsif($comment =~ m!X|(?:Xport)\{$Method_FIELDS+\}!o) {
      my $member = {Type=>$type, Methodbase=>$methodname, ArgStr=>$args, Args=>$ar};
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
      $view =~ s/\(/(Type=>"$type",Methodbase=>"$methodname",Const=>"$const",Args=>"$args",/;
      #print $view."\n";
      $view =~ s/,\)/)/;
      my $control = eval("new GLED::Widgets::"."$view") or die;
      push @MetViews, $control;
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
	$pre   .= "if($r->{Varname}) $r->{Varname}->DecRefCount(); ";      
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
	$post  .= "if($r->{Varname}) $r->{Varname}->IncRefCount(); "      
      }
      $post    .= "mExecMutex.Unlock(); " if $LOCK_SET_METHS;
    }

    $r->{Type} =~ /(.)/; my $arg = lc $1;
    print H7 "void Set$r->{Methodbase}($r->{ArgStr}) {";
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
  if($r->{Xport} =~ m/(e|E)/ && exists $r->{Link} && not($r->{Type} eq "${BASECLASS}*")) {
    print H7<<"fnord";
void Set$r->{Methodbase}(ZGlass* d) {
  $r->{Type} dd=0; if(d) dd = dynamic_cast<$r->{Type}>(d);
  if(d==0 || dd) Set$r->{Methodbase}(dd);
}
fnord
  }

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
print H7 "Int_t E_Exec(TBuffer* buf);\n";
for $r (@Constructors) {
  my $args = "TMessage* _msg" .($r->{ArgStr} ? ", " : ""). $r->{ArgStr};
  print H7 "static TMessage* S_$r->{Methodbase}($args);\n";
}
print H7 "static ${BASECLASS}* Btor(Saturn* _sat_, TBuffer* buf);\n" unless $VirtualBase;

for $r (@Members) {
  next unless $r->{Xport} =~ m/s|S|e|E/o;
  print H7 "ZMIR* S_Set$r->{Methodbase}($r->{ArgStr});\n";
  if(exists $r->{Link}) {
    print H7 "static void SC_Set$r->{Methodbase}(TBuffer* _buf);\n";
  }
}

for $r (@Methods) {
  print H7 "ZMIR* S_$r->{Methodbase}($r->{ArgStr});\n";
  if(exists $r->{Ctx}) {
    print H7 "static void SC_$r->{Methodbase}(TBuffer* _buf);\n";
  }
}
print H7 "\n";

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

print C7 "#include <Stones/ZComet.h>\n";
print C7 "#include <memory>\n\n";

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
    print C7 "    if($r->{Varname}==0) ++ret; else $r->{Varname}->IncRefCount();\n";
    print C7 "  }\n";
  }
  print C7 "  return ret;\n}\n\n";
}

#######################
### Remote-exec methods
#######################

print C7 "#include <Stones/ZMIR.h>\n";
print C7 "#include <Ephra/Saturn.h>\n";
print C7 "#include <TMessage.h>\n\n";

### Constructors ... always static
$methid = 1;
for $r (@Constructors) {
  my $args1 = join(", ", "TMessage* _msg", map( { "$_->[0] $_->[2]" } @{$r->{Args}}));
  # Watch it ... constructors do not stream ZMIR ...
  print C7 <<"fnord";
TMessage*
${CLASSNAME}::S_$r->{Methodbase}($args1) {
  *_msg << (LID_t)$LibID << (CID_t)$ClassID << (MID_t)$methid;
fnord
  print C7 BeamArgs("_msg", $r->{Args});
  print C7 "  return _msg;\n}\n\n";
  $methid++;
}

### Set Methods ...
$methid = 100;
for $r (@Members) {
  next unless $r->{Xport} =~ m/s|S|E|e/o;
  if(exists $r->{Link}) {
    print C7 <<"fnordlink";
ZMIR* ${CLASSNAME}::S_Set$r->{Methodbase}($r->{ArgStr}) {
  ZMIR* _mir = new ZMIR(mSaturnID, ($r->{Args}[0][1] ? $r->{Args}[0][1]\->GetSaturnID() : 0));
  *_mir->Message << (LID_t)$LibID << (CID_t)$ClassID << (MID_t)$methid;
  return _mir;
}
void ${CLASSNAME}::SC_Set$r->{Methodbase}(TBuffer* _buf) {
  *_buf << (LID_t)$LibID << (CID_t)$ClassID << (MID_t)$methid;
}

fnordlink
  } else {
  my $args1 = join(", ", map( { "$_->[0] $_->[2]" } @{$r->{Args}}));
  my $args2 = join(", ", map( { $_->[2] } @{$r->{Args}}));
  print C7 <<"fnord";
ZMIR* ${CLASSNAME}::S_Set$r->{Methodbase}($args1) {
  ZMIR* _mir = new ZMIR(mSaturnID);
  *_mir->Message << (LID_t)$LibID << (CID_t)$ClassID << (MID_t)$methid;
fnord
  print C7 BeamArgs("_mir->Message", $r->{Args});
  print C7 <<"fnord";
  return _mir;
}

fnord
  } # end if Link
  $methid++;
}

### Others/Explicit/Exported/Executable Methods
$methid = 1000;
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
  print C7 "  *_mir->Message << (LID_t)$LibID << (CID_t)$ClassID << (MID_t)$methid;\n";
  my @aa = @{$r->{Args}}[$c .. $C];
  print C7 BeamArgs("_mir->Message", \@aa);
  print C7 <<"fnord";
  return _mir;
}
fnord
  if(exists $r->{Ctx}) {
    print C7 "void\n${CLASSNAME}::SC_$r->{Methodbase}(TBuffer* _buf) {\n";
    print C7 " *_buf << (LID_t)$LibID << (CID_t)$ClassID << (MID_t)$methid;\n}\n";
  }
  print C7 "\n";
  $methid++;
}

############
### E_Exec
############

if($IsGlass) {
  print C7<<"fnord";
Int_t
${CLASSNAME}::E_Exec(TBuffer* buf) {
  static string _eh("${CLASSNAME}::E_Exec ");
  static string _bad_ctx("ctx argument of wrong type");
  MID_t methId; *buf >> methId;
  switch(methId) {
fnord
  # Constructors are NOT handled vie E_Exec; see GledNS::ConstructNode and Btor
  # Set stuff
  $methid = 100;
  for $r (@Members) {
    next unless $r->{Xport} =~ m/s|S|e|E/o;
    if(exists $r->{Link}) {

      print C7 << "fnordlink";
  case $methid: {
    $r->{Type} _beta = dynamic_cast<$r->{Type}>(mMir->Beta);
    if(mMir->Beta != 0 && _beta == 0)
      throw(_eh + "[Set$r->{Methodbase}] " + _bad_ctx);
    Set$r->{Methodbase}(_beta);
    return 0;
  }
fnordlink

    } else {

      print C7 "  case $methid: {\n";
      print C7 QeamArgs($r->{Args}, "    ");
      my @ca = map { $_->[5] } (@{$r->{Args}});
      print C7 "    Set$r->{Methodbase}(". join(", ", @ca) .");\n";
      print C7 "    return 0;\n  }\n";

    } # end if Link
    $methid++;
  }

  # Others
  $methid = 1000;
  for $r (@Methods) {
    print C7 "  case $methid: {\n";
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
      print C7 QeamArgs(\@aa, "    ");
    } else {
      print C7 QeamArgs($r->{Args}, "    ");
    }
    my @ca = map { $_->[5] } (@{$r->{Args}});
    print C7 "    $r->{Methodbase}(". join(", ", @ca) .");";
    print C7 "\n    return 0;\n  }\n";
    $methid++;
  }
  # end
  print C7 " default: { return 16; }\n";
  print C7 " } // end switch\n}\n\n";
}

#### Btor
  if($IsGlass and !$VirtualBase) {
  print C7<<"fnord";
ZGlass*
${CLASSNAME}::Btor(Saturn* _sat_, TBuffer* buf) {
  MID_t methId; *buf >> methId;
  switch(methId) {
  case 0: { return new ${CLASSNAME}; }
fnord
  $methid = 1;
  my $spv = $SUNPTRVAR;
  $SUNPTRVAR = "_sat_";
  for $r (@Constructors) {
    print C7 "  case $methid: {\n";
    print C7 QeamArgs($r->{Args}, "    ");
    my @ca = map { $_->[5] } (@{$r->{Args}});
    print C7 "    return new ${CLASSNAME}(". join(", ", @ca) .");\n  }\n";

    $methid++;
  }
  print C7 " default: { return 0; }\n";
  print C7 " } // end switch\n}\n\n";
  $SUNPTRVAR = $spv;
}

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
  my @incs = map {$_->{Include} } @ExpViews,@VarViews,@MetViews;
  for $r (@incs, @AdditionalViewIncludes) {
    if(not exists $done{$r}) {
      print H "#include <$r>\n";
      $done{$r} = 1;
    }
  }
}

print H "\nclass ${CLASSNAME}View : public MTW_SubView {\n";

for $r (@ExpViews,@VarViews,@MetViews) {
  print H $r->make_header_ccbu();
}

print H <<"fnord";
  ${CLASSNAME}* mIdol;
public:
  ${CLASSNAME}View(GledViewNS::ClassInfo* ci, MTW_View* v, ${CLASSNAME}* i) :
    MTW_SubView(ci, v), mIdol(i) {}

  static void CheckIn();
  static MTW_SubView* Construct(GledViewNS::ClassInfo* ci, MTW_View* v, ZGlass* g);
};

#endif
fnord
close H unless *H==*STDOUT;

########################################################################
# OUT7FILE, option -7file; In fact .cxx file for class <Class>View
########################################################################
gen7:
goto heaven if($OUT7FILE eq 'skip'); # || ($#VarView==-1 and $#MetViews==-1));

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

for $r (@ExpViews,@VarViews,@MetViews) {
  print C $r->make_widget();
  print C $r->make_cxx_cb();
  print C $r->make_weed_update();

  print C "// -----\n\n";
}

######################################
### Check-In into GledViewNS structure
######################################

print C <<"fnord";
static GledViewNS::ClassInfo* _ci=0;

void ${CLASSNAME}View::CheckIn() {
  using namespace GledViewNS;

  if(_ci) return;
  _ci = new ClassInfo(FID_t($LibID, $ClassID));
  _ci->fClassName = "${CLASSNAME}";
  _ci->fooSVCreator = &Construct;
  _ci->fParentName = "$PARENT";
  _ci->fRendererGlass = "$CATALOG->{Classes}{$CLASSNAME}{RnrClass}";
  _ci->fDefRnrCtrl = RnrCtrl(${RnrCtrl_ctor});
  _ci->fRendererCI = 0;
  _ci->fParentCI = 0;
fnord

###################
# ContextMethodInfo
###################

for $r (@Members) {
  if(exists $r->{Link}) {
    print C <<"fnord";
  {
    ContextMethodInfo* cmip = new ContextMethodInfo;
    cmip->fName = "Set$r->{Methodbase}";
    cmip->fContextArgs.push_back("$r->{Args}[0][0] $r->{Args}[0][1]");
    cmip->fooCCCreator = &${CLASSNAME}::SC_Set$r->{Methodbase};

    _ci->fCMIlist.push_back(cmip);
  }
fnord
  }
}

for $r (@Methods) {
  if(exists $r->{Ctx}) {
    print C "  {\n    ContextMethodInfo* cmip = new ContextMethodInfo;\n";
    print C "    cmip->fName = \"$r->{Methodbase}\";\n";
    my $c = substr($r->{Ctx}, 0, 1);
    my $C = $#{$r->{Args}};
    for($i=0; $i<=$C; ++$i) {
      if($i < $c) {
	print C "    cmip->fContextArgs.push_back(\"$r->{Args}[$i][0] $r->{Args}[$i][1]\");\n";
      } else {
	print C "    cmip->fFreeArgs.push_back(\"$r->{Args}[$i][0] $r->{Args}[$i][1]\");\n";
      }
    }
    my $t = substr($r->{Ctx}, 1);
    print C "    cmip->fFreeTemplate = \"$t\";\n";
    print C "    cmip->fooCCCreator = &${CLASSNAME}::SC_$r->{Methodbase};\n";
    print C "\n    _ci->fCMIlist.push_back(cmip);\n  }\n";
  }
}

################
# LinkMemberInfo
################

for $r (@Members) {
  if(exists $r->{Link}) {
    my $is_list = ($r->{Link} =~ m/(l|L)/) ? "true" : "false";
    print C <<"fnord";
  {
    LinkMemberInfo* lmip = new LinkMemberInfo;
    lmip->fName  = "$r->{Methodbase}";
    lmip->fType  = "$r->{Type}";
    lmip->bIsLinkToList = $is_list;
    lmip->fDefRnrBits = RnrBits($r->{RnrBits});
    lmip->fooCCCreator = &${CLASSNAME}::SC_Set$r->{Methodbase};

    _ci->fLMIlist.push_back(lmip);
  }
fnord
  }
}

############
# MemberInfo
############

for $r (@ExpViews,@VarViews,@MetViews) {
  print C <<"fnord";
  {
    MemberInfo* mip = new MemberInfo;
    mip->fName  = "$r->{Methodbase}";
    mip->fType  = "$r->{Type}";
    mip->fWidth = $r->{-width};
    mip->fHeight= $r->{-height};
    mip->bLabel = $r->{LabelP};
    mip->bLabelInside = $r->{LabelInsideP};
    mip->bCanResize = $r->{CanResizeP};
    mip->bJoinNext = $r->{-join};
    mip->fooWCreator  = $r->{Methodbase}_Creator_s;
    mip->fooWCallback = (WeedCallback_foo) $r->{Methodbase}_Callback_s;
    mip->fooWUpdate   = (WeedUpdate_foo)   $r->{Methodbase}_Update_s;

    _ci->fMImap["$r->{Methodbase}"] = mip;
    _ci->fMIlist.push_back(mip);
  }
fnord
}
print C "  GledViewNS::BootstrapClassInfo(_ci);\n}\n\n";

# Creator
print C <<"fnord";
MTW_SubView*
${CLASSNAME}View::Construct(GledViewNS::ClassInfo* ci, MTW_View* v, ZGlass* g) {
  ${CLASSNAME}* tg = dynamic_cast<${CLASSNAME}*>(g);
  if(!tg) return 0;
  return new ${CLASSNAME}View(ci, v, tg);
}
fnord

close C unless *C==*STDOUT;

########################################################################
heaven:
exit(0);
