# $Header$

# Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

=head1 Widgets of Var1 libset

=cut

package GLED::Widgets;

########################################################################

package GLED::Widgets::HPointF; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Type} = "HPointF";
  $S->{Widget} = "Fl_Vector3";
  $S->{Include} = "FL/Fl_Vector3.h";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 28 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S=shift;
  # Tried disble() for -const, but looks too dim.
  # Maybe should really create Fl_ValueOutputs in the Fl_Vector3.
  # Need special constructor then, painful.
  # Idea: display in dark green, all weeds (propagate in Fl_Vector3).
  # Fix also other weeds using that in GLED_Geom1_Widgets.pm.
  return $S->make_widget_A() . $S->make_widget_B();
}

sub make_cxx_cb {
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

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A() .
    "  const HPointF& p = mIdol->Ref$S->{Methodbase}();\n" .
    "  w->set(p.x, p.y, p.z);\n" .
  $S->make_weed_update_B();
}

########################################################################

1;
