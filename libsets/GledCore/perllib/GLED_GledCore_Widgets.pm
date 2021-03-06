# $Id$

# Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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

THIS IS NOT IMPLEMENTED FOR ALL WEEDS ... in callback do update instead.
Maybe it would be better to just deactivate the widget.

make widget read-only. Done on code generation level; nothing is
exported into GlassViewNS::MemberInfo structure. Perhaps the other way
around would be smarter ... 

And ... the update should go via MTW_View, where locking is performed!!!
Now it is not (for ValOut and BoolOut)

=item B<< -tooltip=>"some text" >>

Obvious.
Not implemented for MCWButt and Link as they generate tooltip on the fly.
Maybe should override or combine the two.

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
  $S->{IsLinkWeed} = "false";
  return $S;
}

# need several makes; some pureley optional (like range settings)
# consider A,B like {,} [ or begin/end ]
sub make_widget_A {
  my $S = shift;
  return "Fl_Widget* ${::CLASSNAME}View::$S->{Methodbase}_Creator() {\n" .
    "  $S->{Widget}* o = new $S->{Widget}(0,0,0,0,\"$S->{Methodbase}\");\n";
}

sub make_widget_ttip {
  my $S = shift;
  return (exists $S->{-tooltip}) ? "  o->tooltip(\"$S->{-tooltip}\");\n" : "";
}

sub make_widget_B {
  my $S = shift;
  return 
    "  o->callback((Fl_Callback*)$S->{Methodbase}_Callback_s, this);\n" .
    $S->make_widget_ttip() .
    "  return o;\n" . "}\n\n";
}

###############
sub make_widget
###############
{
  my $S = shift;
  return $S->make_widget_A() . $S->make_widget_B();
}

##############
sub make_range
##############
{
  my $S = shift;
  if(exists $S->{-range}) {
    my $rr = $S->{-range};
    my $steparg = "$rr->[2]";
    $steparg .= ",$rr->[3]" if defined $rr->[3];
    return "  o->minimum($rr->[0]); o->maximum($rr->[1]); o->step($steparg);\n";
  } else {
    return "";
  }
}

##############
sub make_step
##############
{
  my $S = shift;
  if(exists $S->{-range}) {
    my $rr = $S->{-range};
    my $steparg = "$rr->[2]";
    $steparg .= ",$rr->[3]" if defined $rr->[3];
    return "  o->step($steparg);\n";
  } else {
    return "";
  }
}

# make_widget_type would come handy ... for groups, [] of types ...
# like o->type(FL_HOR_FILL_SLIDER);

########################################################################

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

########################################################################

sub make_var_widget_cb
{
  my $S = shift;

  return $S->make_varout_widget_cb() if $S->{-const};

  my $valuestr = (not exists $S->{CastTo}) ? "o->value()" : "(($S->{CastTo})(o->value()))";
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

sub make_varout_widget_cb
{
# varout cb only does update to true value
  my $S = shift;
  my $r = $S->make_weed_update();
  $r =~ s/_Update/_Callback/o;
  return $r;
}

sub make_text_widget_cb
{
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

sub make_met_widget_cb
{
# missing handling of arguments
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
  my $w = $l1 > $l2 ? $l1 : $l2;
  if(defined $S->{-range}[3]) {
    my $m1 = length($S->{-range}[2]/$S->{-range}[3]) - 2; # subtract '0.'
    $w += $m1 if $m1 > 0;
  }
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

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_Value_Input";
  $S->{Include} = "FL/Fl_Value_Input.H";
  $S->{CastTo} = "double";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = exists $S->{-width} ? "false" : "true";
  $S->{-width} = $S->measure_range() unless exists $S->{-width};
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

package GLED::Widgets::HexValue; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget}  = "Fl_Int_Input";
  $S->{Include} = "FL/Fl_Int_Input.H";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = exists $S->{-width} ? "false" : "true";
  $S->{-width} = $S->measure_range() unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  $S->{-format} = '0x%x' unless exists $S->{-format};
  return $S;
}

sub make_widget {
  my $S = shift;
  return $S->make_widget_A() . "  o->when(FL_WHEN_CHANGED);\n" .
         $S->make_widget_B();
}

sub make_cxx_cb {
  my $S = shift;
  return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  int _val; sscanf(o->value(), "%x", &_val);
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_Set$S->{Methodbase}(_val) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->Set$S->{Methodbase}(_val);
  }
}\n
fnord
}

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A() .
"  int _val = mIdol->Get$S->{Methodbase}();\n" .
"  int _exval; sscanf(w->value(), \"%x\", &_exval);\n" .
"  if(_val != _exval) w->value(GForm(\"$S->{-format}\", _val));\n" .
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

package GLED::Widgets::MinMaxVar; @ISA = ('GLED::Widgets');

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget}  = "Fl_SMinMaxVar";
  $S->{Include} = "FL/Fl_SVars.h";
  # $S->{CastTo}  = "double";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = exists $S->{-width} ? "false" : "true";
  $S->{-width}  = 8 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget
{
  my $S = shift;
  $S->make_widget_A() . $S->make_range() . $S->make_widget_B();
}

sub make_cxx_cb
{
  my $S = shift;
  $S->make_var_widget_cb();
}

sub make_weed_update
{
  my $S = shift;
  $S->make_weed_update_A() .
"  const $S->{Type}& v = mIdol->Ref$S->{Methodbase}();\n" .
"  w->set(v.Get(), v.GetMin(), v.GetMax());\n" .
  $S->make_weed_update_B();
}

########################################################################

package GLED::Widgets::InertVar; @ISA = ('GLED::Widgets::MinMaxVar');

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget}  = "Fl_SInertVar";
  $S->{Include} = "FL/Fl_SVars.h";
  # $S->{CastTo}  = "double";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = exists $S->{-width} ? "false" : "true";
  $S->{-width}  = 8 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget
{
  my $S = shift;
  $S->make_widget_A() . $S->make_step() . $S->make_widget_B();
}

########################################################################

package GLED::Widgets::DesireVar; @ISA = ('GLED::Widgets::InertVar');

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget}  = "Fl_SDesireVar";
  $S->{Include} = "FL/Fl_SVars.h";
  # $S->{CastTo}  = "double";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = exists $S->{-width} ? "false" : "true";
  $S->{-width}  = 8 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_weed_update
{
  my $S = shift;
  $S->make_weed_update_A() .
"  const $S->{Type}& v = mIdol->Ref$S->{Methodbase}();\n" .
"  w->set(v.Get(), v.GetMin(), v.GetMax(), v.GetDesire());\n" .
"  if (!v.IsDesireSatisfied()) SetUpdateTimer();\n" .
  $S->make_weed_update_B();
}

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
  my $whencb;
  if(defined $S->{-whenchanged}) {
    $whencb = "  o->when(FL_WHEN_CHANGED);\n";
  } else {
    $whencb = "  o->when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);\n" .
      "  o->color(fl_rgb_color(255, 245, 245));\n" .
      "  o->tooltip(sTextorTooltip);\n";
  }
  return $S->make_widget_A() . $whencb . $S->make_widget_B();
}

sub make_cxx_cb { my $S = shift; $S->make_text_widget_cb(); }

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A().
"  if(strcmp(w->value(), mIdol->Get$S->{Methodbase}()) != 0) {
    w->value(mIdol->Get$S->{Methodbase}());
    if(w->position() != w->mark()) w->position(w->size());
   }\n".
  $S->make_weed_update_B();
}

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
  if(defined $S->{-whenchanged}) {
    $whencb = "  o->when(FL_WHEN_CHANGED);\n";
  } else {
    $whencb = "  o->when(FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);\n" .
      "  o->color(fl_rgb_color(255, 245, 245));\n" .
      "  o->tooltip(sTextorTooltip);\n";
  }
  return $S->make_widget_A() . $whencb .
    "\to->tooltip(\"R-button opens file selector\");\n" .
    (defined $S->{-pat} ? "\to->pattern(\"$S->{-pat}\");\n" : "" ) .
    (defined $S->{-abs} ? "\to->absolute_p(true);\n" : "" ) .
    (defined $S->{-dir} ? "\to->directory_p(true);\n" : "" ) .
    $S->make_widget_B();
}

sub make_cxx_cb { my $S = shift; $S->make_text_widget_cb(); }

########################################################################

package GLED::Widgets::PhonyEnum; @ISA = ('GLED::Widgets');

# Give explicit values and labels:
# -vals    [(<val>,<label>)*]
# -seqvals [<labels>] values ascending from zero with given labels
# The largest label width + 2 is taken for widget width.

# Otherwise auto deduce from class catalog based on $S->{Type} or -type
# -type    <string>     - specify enum type for catalog search
# -names   [<string>*]  - only show specified names
# The maximum width of all labels +2 is taken for widget width.

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "Fl_PhonyEnum";
  $S->{Include} = "FL/Fl_PhonyEnum.h";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width}  = 0 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};

  $S->{calced_width} = 0;

  return $S;
}

sub check_calced_width
{
  my $S   = shift;
  my $str = shift;
  my $w = length $str;
  $S->{calced_width} = $w if $w > $S->{calced_width};
}

sub enum_details
{
  my $S = shift;
  my $E = {};
  my $_eh = "GLED::Widgets::PhonyEnum::enum_details ";

  if (defined $S->{-vals} or defined $S->{-seqvals}) {
    $E->{EnumCastType} = $S->{Type};
  } else {
    my $type  = (defined $S->{-type}) ? $S->{-type} : $S->{Type};
    my ($class, $enum) = $type =~ m/(\w+)(?:::(\w+))?/;
    # print "For type='$type' got class='$class', enum='$enum'\n";
    unless(defined $enum) { $enum = $class; $class = $::CLASSNAME; }

    if (defined $S->{-type}) {
      $E->{EnumCastType} = $S->{Type};
    } else {
      $E->{EnumCastType} = $class .'::'. $enum;
    }
    $E->{Enum}      = $enum;
    $E->{EnumClass} = $class;
  }
  return $E;
}

sub make_widget
{
  my $S = shift;
  my $eh = "GLED::Widgets::PhonyEnum ($S->{Methodbase})";
  my $r = $S->make_widget_A();

  if($S->{-const}) {
    $r .= "  o->labelcolor(fl_color_cube(2,0,0));\n";
  }

  if (defined $S->{-vals})
  {
    die "$eh uneven number of arguments for -vals"
      if ($#{$S->{-vals}} + 1) % 2 != 0;
    for ($i=0; $i<$#{$S->{-vals}}; $i+=2)
    {
      $r .= "  o->AddEntry($S->{-vals}[$i], \"$S->{-vals}[$i+1]\");\n";
      $S->check_calced_width($S->{-vals}[$i+1]);
    }
    if ($S->{-width} == 0)
    {
      $S->{-width} = $S->{calced_width} + 2
    }
  }
  elsif (defined $S->{-seqvals})
  {
    for ($i=0; $i<=$#{$S->{-seqvals}}; ++$i)
    {
      $r .= "  o->AddEntry($i, \"$S->{-seqvals}[$i]\");\n";
      $S->check_calced_width($S->{-seqvals}[$i]);
    }
    if ($S->{-width} == 0)
    {
      $S->{-width} = $S->{calced_width} + 2
    }
  }
  else
  {
    my $E = $S->enum_details();
    $r .= <<"fnord";
  GledNS::ClassInfo *ci = GledNS::FindClassInfo("$E->{EnumClass}");
  GledNS::EnumInfo  *ei = ci->FindEnumInfo("$E->{Enum}", true);
fnord
    if (defined $S->{-names} and ref($S->{-names})  eq "ARRAY")
    {
      $r .= "  { GledNS::EnumEntry *ee;\n";
      for $n (@{$S->{-names}})
      {
	$r .= "    ee = ei->FindEntry(\"$n\");\n";
	$r .= "    o->AddEntry(ee->fValue, ee->fLabel);\n";
      }
      $r .= "  }\n";
    }
    else
    {
      $r .= "  for (GledNS::vEnumEntry_i i=ei->fEntries.begin(); i!=ei->fEntries.end(); ++i)\n";
      $r .= "  { o->AddEntry(i->fValue, i->fLabel); }\n";
    }
    if ($S->{-width} == 0)
    {
      $S->{-width} = "GledNS::FindClassInfo(\"$E->{EnumClass}\")->FindEnumInfo(\"$E->{Enum}\", true)->fMaxLabelWidth + 2";
    }
  }
  $r .= $S->make_widget_B();

  $r;
}

sub make_cxx_cb
{
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

sub make_weed_update
{
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
# no args ... 
package GLED::Widgets::MCWButt; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} = "MCW_Button";
  $S->{Include} = "GledView/MCW_Button.h";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "true";
  $S->{CanResizeP}   = "true";
  $S->{-width}  = 4 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S = shift;
  return <<"fnord";
Fl_Widget* ${::CLASSNAME}View::$S->{Methodbase}_Creator() {
  static MCW_Button::Data $S->{Methodbase}_butt_data;
  if($S->{Methodbase}_butt_data.fLabel.IsNull()) {
    $S->{Widget}::FillData(${::CLASSNAME}::GlassInfo()->FindMethodInfo($S->{TOP}{ID}),
                            \"$S->{Methodbase}\", $S->{Methodbase}_butt_data);
  }
  $S->{Widget}* o = new $S->{Widget}(mView->fImg, $S->{Methodbase}_butt_data,
                                     0,0,0,0);
  return o;
}\n
fnord
}

sub make_cxx_cb {
  my $S = shift;
  return "void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {}\n";
}

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
         "  o->label(0);\n" .
	 $S->make_widget_B();
}

sub make_cxx_cb {
  my $S = shift;
return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_SetPos(o->x(),o->y(),o->z()) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->SetPos(o->x(),o->y(),o->z());
  }
}\n
fnord
}

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A() .
    "  Double_t x[3]; mIdol->Ref$S->{Transname}().GetPos(x);\n" .
    "  w->pos(x[0],x[1],x[2]);\n" .
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
         "  o->label(0);\n" .
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
    "  Float_t x[3];  mIdol->Ref$S->{Transname}().GetRotAngles(x);\n" .
    "  w->rot(x[0],x[1],x[2]);\n" .
  $S->make_weed_update_B();
}

########################################################################

package GLED::Widgets::HPointBase; @ISA = ('GLED::Widgets');

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Type} = "unknown"; # overridden in sub-classes
  $S->{Widget} = "Fl_Vector3";
  $S->{Include} = "FL/Fl_Vector3.h";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 28 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget
{
  my $S=shift;
  # Tried disble() for -const, but looks too dim.
  # Maybe should really create Fl_ValueOutputs in the Fl_Vector3.
  # Need special constructor then, painful.
  # Idea: display in dark green, all weeds (propagate in Fl_Vector3).
  # Fix also other weeds using that in GLED_Geom1_Widgets.pm.
  return $S->make_widget_A() . $S->make_widget_B();
}

sub make_cxx_cb
{
  my $S = shift;
  my $r = "void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {\n";
  if($S->{-const}) {
    $r .= "  $S->{Methodbase}_Update(o)\n;";
  } else {
    $r .= <<"fnord";
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_Set$S->{Methodbase}(o->x(),o->y(),o->z()) );
    e->Send(*_m);
    SetUpdateTimer();
  } else {
    mIdol->Set$S->{Methodbase}(o->x(),o->y(),o->z());
  }
fnord
  }
  return $r . "}\n\n";
}

sub make_weed_update
{
  my $S = shift;
  $S->make_weed_update_A() .
    "  const $S->{Type}& p = mIdol->Ref$S->{Methodbase}();\n" .
    "  w->set(p.x, p.y, p.z);\n" .
  $S->make_weed_update_B();
}

#-----------------------------------------------------------------------

package GLED::Widgets::HPointF; @ISA = ('GLED::Widgets::HPointBase');

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Type} = "HPointF";
  return $S;
}

#-----------------------------------------------------------------------

package GLED::Widgets::HPointD; @ISA = ('GLED::Widgets::HPointBase');

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Type} = "HPointD";
  return $S;
}

#-----------------------------------------------------------------------

package GLED::Widgets::ZPoint; @ISA = ('GLED::Widgets::HPointBase');

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Type} = "ZPoint";
  return $S;
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
    $S->make_widget_ttip() .
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

package GLED::Widgets::StoneOutput; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} =  "Fl_Output";
  $S->{Include} = "FL/Fl_Output.H";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} =  12 unless exists $S->{-width};
  $S->{-height} = 1  unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S=shift;
  return $S->make_widget_A() .
	 $S->make_widget_B();
}

sub make_cxx_cb {
  my $S = shift;
return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {}

fnord
}

sub make_weed_update {
  my $S = shift;
  return $S->make_weed_update_A() .
    "  w->value(GForm(\"$S->{Fmt}\", " . 
      join(', ', map { "mIdol->Ref$S->{Methodbase}().Get$_()" } @{$S->{Args}}) .
	"));\n" .
  $S->make_weed_update_B();
}

########################################################################

package GLED::Widgets::TimeOut; @ISA = ('GLED::Widgets');

# Shows GTime as text, specify Cmd argument as:
#     To[Asc|DateTime][GMT|Local]([false|true])
# These should really be options as static data-members in a glass, a set of
# them (Fmt, TZ, ShowTZ) for every GTime that uses them. But would need a
# proper Fl widget then.

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Widget} =  "Fl_Output";
  $S->{Include} = "FL/Fl_Output.H";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width}  = 20 unless exists $S->{-width};
  $S->{-height} =  1 unless exists $S->{-height};
  $S->{Cmd} = "ToDateTimeLocal()" unless exists $S->{Cmd};
  return $S;
}

sub make_widget
{
  my $S=shift;
  return $S->make_widget_A() .
	 $S->make_widget_B();
}

sub make_cxx_cb
{
  my $S = shift;
return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {}

fnord
}

sub make_weed_update
{
  my $S = shift;
  return $S->make_weed_update_A() .
    "  w->value(mIdol->Ref$S->{Methodbase}().$S->{Cmd});\n" .
  $S->make_weed_update_B();
}

########################################################################

package GLED::Widgets::Link; @ISA = ('GLED::Widgets');

sub new
{
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{IsLinkWeed} = "true";
  $S->{Widget} = "FltkGledStuff::LinkNameBox";
  $S->{Include} = "GledView/FltkGledStuff.h";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 12 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget
{
  my $S = shift;
  my $fqn = "${::CLASSNAME}::$S->{Methodbase}";
  my $link_type = $S->{Type};

  $link_type =~ s/\*//;

  return <<"fnord"
Fl_Widget* ${::CLASSNAME}View::$S->{Methodbase}_Creator() {
  OptoStructs::ZLinkDatum* ld = GrepLinkDatum(\"$fqn\");
  if(ld == 0) return 0;
  $S->{Widget}* o = new $S->{Widget}(ld, 0,0,0,0,\"$S->{Methodbase}\");
  o->fFID = GledNS::FindClassID(\"$link_type\");
  return o;
}\n
fnord
}

sub make_cxx_cb
{
  my $S = shift;
  return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o)
{}\n
fnord
}

sub make_weed_update
{
  my $S = shift;
  my $x = "  if(w->LinkViewNeedsUpdate()) w->LinkViewUpdate();\n";

  return $S->make_weed_update_A() . $x . $S->make_weed_update_B();
}

########################################################################

1;
