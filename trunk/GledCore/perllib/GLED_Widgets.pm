# $Header$

# Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

=head1 WHAT SHOULD BE PROVIDED BY A WIDGET CLASS

C<project7> instantiates these objects by expanding widget description
part of the comment with C<Type> and C<Methodbase>, then it simply evals the
resulting statement. These classes should produce code needed for a widget to
be created and used (callbacks, updates) in a GLED environment.

Even though the code can be anything (as long as it compiles), three pairs of
static/member functions must be defined:

B<Creation> C<< <Methodbase>_Creator[_s] >>; member foo should also set callbacks

B<Callback> C<< <Methodbase>_Callback[_s] >>

B<Update> C<< <Methodbase>_Update[_s] >>


In process of producing C<GlassView>, several methods are called.

=head2 Producing C<.h> file

Values of C<< $weed->Include >> are uniqued and C<#include> statements produced.

C<< $weed->make_header_ccbu() >> is called for all weeds. Return values are directly
inserted in the C<private> section of a GlassView. Each C<$weed> should declare all
symbols that will be defined in the C<.cxx> file.

=head2 Producing C<.cxx> file

C<< $weed->make_widget() >> 

C<< $weed->make_cxx_cb() >>

C<< $weed->make_weed_update() >>


=head1 WEED PROPERTIES

=over 4

=item B<LabelP>

the weed has a visible label (its name, or in p7 lingo C<Methodbase> is used as label)

=item B<LabelInsideP>

the label is inside the widget (like in buttons)

=item B<CanResizeP>

the weed can be resized (some weeds are fixed size and would react badly to resizing)

=item B<-width>

minimal width of the weed. If labeled on the inside this is the minimal width WITHOUT
the label: it is taken into account upon construction.

=item B<< -join=>1 >>

when putting the widget into a full-lens-view attempt to join the widget
on previous line.

=item B<< -const=>1 >>

THIS IS NOT IMPLEMENTED ... but would be nice in one form or another. 

make widget read-only. Done on code generation level; nothing is
exported into GlassViewNS::MemberInfo structure. Perhaps the other way
around would be smarter ... 

And ... the update should go via MTW_View, where locking is performed!!!
Now it is not (for ValOut and BoolOut)

=back

=cut

package GLED::Widgets;

sub new {
  my $proto = shift;
  my $class = ref($proto) || $proto;
  my $S = {@_};
  bless($S, $class);

  $S->{-join}  = 0 unless exists $S->{-join};
  $S->{-const} = 0 unless exists $S->{-const};
  return $S;
}

# need several makes; some pureley optional (like range settings)
# consider A,B like {,} [ or begin/end ]
sub make_widget_A {
  my $S = shift;
  return "Fl_Widget* ${::CLASSNAME}View::$S->{Methodbase}_Creator() {\n" .
    "  $S->{Widget}* o = new $S->{Widget}(0,0,0,0,\"$S->{Methodbase}\");\n";
}

sub make_widget_B {
  my $S = shift;
  return "  o->callback((Fl_Callback*)$S->{Methodbase}_Callback_s, this);\n" .
    "  return o;\n" . "}\n\n";
}

#################
sub make_widget {
#################
  my $S = shift;
  return $S->make_widget_A() . $S->make_widget_B();
}

sub make_range {
  my $S = shift;
  if(exists $S->{-range}) {
    $rr = $S->{-range};
    my $steparg = "$rr->[2]";
    $steparg .= ",$rr->[3]" if defined $rr->[3];
    return "  o->minimum($rr->[0]); o->maximum($rr->[1]); o->step($steparg);\n";
  } else {
    return "";
  }
}

# make_widget_type would come handy ... for groups, [] of types ...
# like o->type(FL_HOR_FILL_SLIDER);

########################################################################
# cb_base foos

######################
sub make_header_ccbu {
######################
  # same for vars and mets
  my $S = shift;
  return <<"fnord";
  static Fl_Widget* $S->{Methodbase}_Creator_s(MTW_SubView* v) {
    return ((${::CLASSNAME}View*)v)->$S->{Methodbase}_Creator();
  }
  Fl_Widget* $S->{Methodbase}_Creator();

  static void $S->{Methodbase}_Callback_s($S->{Widget}* w, ${::CLASSNAME}View* v) {
    v->$S->{Methodbase}_Callback(w);
  }
  void $S->{Methodbase}_Callback($S->{Widget}*);

  static void $S->{Methodbase}_Update_s(Fl_Widget* w, MTW_SubView* v) {
    ((${::CLASSNAME}View*)v)->$S->{Methodbase}_Update(($S->{Widget}*)w);
  }
  void $S->{Methodbase}_Update($S->{Widget}* w);

  // -----

fnord
}

sub make_var_widget_cb {
  my $S = shift;
  my $valuestr = (not exists $S->{CastTo}) ? "o->value()" : "(($S->{Type})(o->value()))";
  return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_Set$S->{Methodbase}($valuestr) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->Set$S->{Methodbase}($valuestr);
  }
}\n
fnord
}

sub make_varout_widget_cb {
# varout cb only does update to true value
  my $S = shift;
  my $r = $S->make_weed_update();
  $r =~ s/_Update/_Callback/o;
  return $r;
}

sub make_text_widget_cb {
  my $S = shift;
  $valuestr = (not exists $S->{CastTo}) ? "o->value()" : "(($S->{Type})(o->value()))";
  return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_Set$S->{Methodbase}($valuestr) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->Set$S->{Methodbase}($valuestr);
  }
}\n
fnord
}

sub make_met_widget_cb {
# missing handling of arguments ... perhaps should export Node to Cint, or sth
  my $S = shift;
  return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_$S->{Methodbase}() );
    e->Send(*_m);
  } else {
    mIdol->$S->{Methodbase}();
  }
}\n
fnord

}

########################################################################
# update foos

sub make_weed_update_A {
  my $S = shift;
  "void ${::CLASSNAME}View::$S->{Methodbase}_Update($S->{Widget}* w) {\n";
}

sub make_weed_update_B {
  my $S = shift;
  "}\n\n";
}

######################
sub make_weed_update {
######################
  my $S = shift;
  if(not exists $S->{CastTo}) {
    $prestr = ""; $poststr = "";
  } else {
    $prestr = "($S->{CastTo})("; $poststr = ")";
  }
  $S->make_weed_update_A().
"  w->value(${prestr}mIdol->Get$S->{Methodbase}()${poststr});\n".
  $S->make_weed_update_B();
}

########################################################################
# pure helpers

sub measure_range {
  # !! not really smart ...
  my $S = shift;
  return 4 unless exists $S->{-range};
  my ($l1, $l2) = ( length(eval($S->{-range}[0])), length(eval($S->{-range}[1])) );
  my $m1 = defined $S->{-range}[3] ? length($S->{-range}[2]/$S->{-range}[3]) :
    length($S->{-range}[2]);
  my $w = $l1 > $l2 ? $l1 : $l2;
  $w += $m1-2 if $m1 > 2;
  return $w + 1;
}

########################################################################
# True widget classes
########################################################################

package GLED::Widgets::Bool; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_Light_Button";
  $S->{Include} = "FL/Fl_Light_Button.H";
  $S->{CastTo} = "int"; # internal representation of value() for button
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "true";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 3 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_cxx_cb { my $S = shift; $S->make_var_widget_cb(); }

########################################################################

package GLED::Widgets::BoolOut; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_Light_Button";
  $S->{Include} = "FL/Fl_Light_Button.H";
  $S->{CastTo} = "int"; # internal representation of value() for button
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "true";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 3 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S = shift;
  return $S->make_widget_A() . "  o->labelcolor(fl_color_cube(2,0,0));\n" . $S->make_widget_B();
}
sub make_cxx_cb { my $S = shift; $S->make_varout_widget_cb(); }

########################################################################

package GLED::Widgets::Value; @ISA = ('GLED::Widgets');

sub xxxx { print "xxxx\n";}

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_Value_Input";
  $S->{Include} = "FL/Fl_Value_Input.H";
  $S->{CastTo} = "double";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = $S->measure_range()  unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S = shift;
  return $S->make_widget_A() . $S->make_range() . $S->make_widget_B();
}

sub make_cxx_cb { my $S = shift; $S->make_var_widget_cb(); }

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A() .
"  $S->{Type} _val = mIdol->Get$S->{Methodbase}();\n" .
"  if(_val != ($S->{Type})w->value()) w->value(_val);\n" .
  $S->make_weed_update_B();
}

########################################################################

package GLED::Widgets::ValOut; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_Value_Output";
  $S->{Include} = "FL/Fl_Value_Output.H";
  $S->{CastTo} = "double";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  if (exists $S->{-width}) {
    $S->{CanResizeP}   = "false";
  } else {
    $S->{-width} = $S->measure_range();    
    $S->{CanResizeP}   = "true";
  }
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S = shift;
  return $S->make_widget_A() . $S->make_range() . $S->make_widget_B();
}

sub make_cxx_cb { my $S = shift; $S->make_varout_widget_cb(); }

########################################################################

package GLED::Widgets::Textor; @ISA = ('GLED::Widgets');
# works on TString class

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_Input";
  $S->{Include} = "FL/Fl_Input.H";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 12 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S = shift;
  return $S->make_widget_A() . 
    #"\to->when(FL_WHEN_ENTER_KEY|FL_WHEN_RELEASE);\n" .
    $S->make_widget_B();
}

sub make_cxx_cb { my $S = shift; $S->make_text_widget_cb(); }

########################################################################

package GLED::Widgets::TextOut; @ISA = ('GLED::Widgets');
# works on TString class

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_Output";
  $S->{Include} = "FL/Fl_Output.H";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 12 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S = shift;
  return $S->make_widget_A() .
    "\to->when(0);\n" .
    $S->make_widget_B();
}

sub make_cxx_cb { my $S = shift; $S->make_varout_widget_cb(); }

########################################################################

package GLED::Widgets::Filor; @ISA = ('GLED::Widgets');
# works on TString class
# !!!! should use fltk-mt

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_File_Name";
  $S->{Include} = "FL/Fl_File_Name.H";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 16 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S = shift;
  return $S->make_widget_A() .
    "\to->when(FL_WHEN_ENTER_KEY|FL_WHEN_RELEASE);\n" .
    $S->make_widget_B();
}

sub make_cxx_cb { my $S = shift; $S->make_text_widget_cb(); }

########################################################################

package GLED::Widgets::PhonyEnum; @ISA = ('GLED::Widgets');

# -vals    [(<val>,<label>)*]
# -seqvals [<labels>] values from zero with given label
# Otherwise auto deduce from catalog based on $S->{Type} or -type
# -type    <string>
# -labels  [<string>*]
# -names   [<string>*]

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_PhonyEnum";
  $S->{Include} = "FL/Fl_PhonyEnum.h";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 16 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};

  return $S;
}

sub enum_details {
  my $S = shift;
  my $E = {};
  my $_eh = "GLED::Widgets::PhonyEnum::enum_details ";

  if(defined $S->{-vals} or defined $S->{-seqvals}) {
    $E->{EnumCastType} = $S->{Type};
  } else {
    my $type  = (defined $S->{-type}) ? $S->{-type} : $S->{Type};
    my ($class, $enum) = $type =~ m/(\w+)(?:::(\w+))?/;
    # print "For type='$type' got class='$class', enum='$enum'\n";
    unless(defined $enum) { $enum = $class; $class = $::CLASSNAME; }
    die "$_eh resolver entry not present for enum ${class}::${enum}"
      unless defined $::resolver->{'GlassName2GlassSpecs'}{$class}{'Enums'}{$enum};
    if(defined $S->{-type}) {
      $E->{EnumCastType} = $S->{Type};
    } else {
      $E->{EnumCastType} = $class .'::'. $enum;
    }
    $E->{EnumSrcClass} = $class;
    $E->{EnumInfo}  = $::resolver->{'GlassName2GlassSpecs'}{$class}{'Enums'}{$enum};
  }
  return $E;
}

sub make_widget {
  my $S = shift;
  my $eh = "GLED::Widgets::PhonyEnum ($S->{Methodbase})";
  my $r = $S->make_widget_A();

  if($S->{-const}) {
    $r .= "  o->labelcolor(fl_color_cube(2,0,0));\n";
  }

  if(defined $S->{-vals}) {
    die "$eh uneven number of arguments for -vals"
      if ($#{$S->{-vals}} + 1) % 2 != 0;
    for($i=0; $i<$#{$S->{-vals}}; $i+=2) {
      $r .= "  o->Bruh($S->{-vals}[$i], \"$S->{-vals}[$i+1]\");\n";
    }

  } elsif(defined $S->{-seqvals}) {
    for($i=0; $i<=$#{$S->{-seqvals}}; ++$i) {
      $r .= "  o->Bruh($i, \"$S->{-seqvals}[$i]\");\n";
    }

  } else {
    my $E = $S->enum_details();
    my $labp = defined $S->{-labels} and ref($S->{-labels}) eq "ARRAY";
    my $namp = defined $S->{-names}  and ref($S->{-names})  eq "ARRAY";
    for $h (@{$E->{EnumInfo}}) {
      my $add = 0;
      if($labp or $namp) {
	$add = 1 if(($labp and grep $h->{label}, @{$S->{-labels}}) or
		    ($namp and grep $h->{name},  @{$S->{-names}}));
      } else {
	$add = 1;
      }
      $r .= "  o->Bruh($E->{EnumSrcClass}::$h->{name}, \"$h->{label}\");\n" if $add;
    }
  }
  $r .= $S->make_widget_B();
  $r;
}


sub make_cxx_cb {
  my $S = shift;
  my $E = $S->enum_details();
  my $r = "void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {\n";
  if($S->{-const}) {
    $r .= "  $S->{Methodbase}_Update(o)\n;";
  } else {
    $r .= <<"fnord";
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_Set$S->{Methodbase}(($E->{EnumCastType})(o->GetTrueVal())) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->Set$S->{Methodbase}(($E->{EnumCastType})(o->GetTrueVal()));
  }
fnord
  }
  $r .= "}\n\n";
  return $r;
}

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A().
  "  w->Update(mIdol->Get$S->{Methodbase}());\n".
  $S->make_weed_update_B();
}

########################################################################
# no args ...
package GLED::Widgets::MButt; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_Button";
  $S->{Include} = "FL/Fl_Button.H";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "true";
  $S->{CanResizeP}   = "true";
  $S->{-width}  = 2 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_cxx_cb { my $S = shift; $S->make_met_widget_cb(); }
sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A().
  $S->make_weed_update_B();
}

########################################################################

package GLED::Widgets::Trans_Pos_Ctrl; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Type} = "ZTrans";
  $S->{Widget} = "Fl_TransPosCtrl";
  $S->{Include} = "FL/Fl_TransCtrl.h";
  $S->{LabelP}       = "false";
  $S->{LabelInsideP} = "true";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 24 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S=shift;
  return $S->make_widget_A() .
         "\to->labeltype(FL_NO_LABEL);\n" .
	 $S->make_widget_B();
}

sub make_cxx_cb {
  my $S = shift;
return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_Set3Pos(o->x(),o->y(),o->z()) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->Set3Pos(o->x(),o->y(),o->z());
  }
}\n
fnord
}

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A() .
    "  ZVec3 x = mIdol->Ref$S->{Transname}().Get3Pos();\n" .
    "  w->pos(x(0),x(1),x(2));\n" .
  $S->make_weed_update_B();
}

########################################################################

package GLED::Widgets::Trans_Rot_Ctrl; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_TransRotCtrl";
  $S->{Include} = "FL/Fl_TransCtrl.h";
  $S->{LabelP}       = "false";
  $S->{LabelInsideP} = "true";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 24 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S=shift;
  return $S->make_widget_A() .
         "\to->labeltype(FL_NO_LABEL);\n" .
	 $S->make_widget_B();
}

sub make_cxx_cb {
  my $S = shift;
return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_SetRotByAngles(o->phi(),o->theta(),o->eta()) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->SetRotByAngles(o->phi(),o->theta(),o->eta());
  }
}\n
fnord
}

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A().
    "  ZVec3 x = mIdol->Ref$S->{Transname}().Get3Rot();\n" .
    "  w->rot(x(0),x(1),x(2));\n" .
  $S->make_weed_update_B();
}

########################################################################

package GLED::Widgets::ColorButt; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_RGBA_Button";
  $S->{Include} = "FL/Fl_RGBA_Button.H";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} =  4 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S = shift;
  return $S->make_widget_A() .
    "\to->on_change( (Fl_Callback*)$S->{Methodbase}_Callback_s, this );\n" .
    "\to->box(FL_EMBOSSED_FRAME);\n" .
    "  return o;\n" . "}\n\n";
}

sub make_cxx_cb {
  my $S = shift;
  return <<"fnord"
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_Set$S->{Methodbase}(o->r,o->g,o->b,o->a) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->Set$S->{Methodbase}(o->r,o->g,o->b,o->a);
    o->redraw();
  }
}\n
fnord
}

sub make_weed_update {
  my $S = shift;
$S->make_weed_update_A().
qq(  const ZColor* _col = mIdol->Ptr$S->{Methodbase}();
  w->set_rgba(_col->r(),_col->g(),_col->b(),_col->a());
).
$S->make_weed_update_B();
}

########################################################################

1;
