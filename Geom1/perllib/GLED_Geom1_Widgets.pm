# $Header$

# Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
# This file is part of GLED, released under GNU General Public License version 2.
# For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

=head1 Widgets of Geom1 libset

=cut

package GLED::Widgets;

########################################################################

package GLED::Widgets::LorentzVector; @ISA = ('GLED::Widgets');

sub new {
  my $proto = shift;
  my $S = $proto->SUPER::new(@_);
  $S->{Type} = "TLorentzVector";
  $S->{Widget} = "Fl_LorentzVector";
  $S->{Include} = "FL/Fl_LorentzVector.h";
  $S->{LabelP}       = "true";
  $S->{LabelInsideP} = "false";
  $S->{CanResizeP}   = "true";
  $S->{-width} = 28 unless exists $S->{-width};
  $S->{-height} = 1 unless exists $S->{-height};
  return $S;
}

sub make_widget {
  my $S=shift;
  return $S->make_widget_A() . $S->make_widget_B();
}

sub make_cxx_cb {
  my $S = shift;
return <<"fnord";
void ${::CLASSNAME}View::$S->{Methodbase}_Callback($S->{Widget}* o) {
  Eye* e = (mView->fImg) ? mView->fImg->fEye : 0;
  if(e) {
    auto_ptr<ZMIR> _m( mIdol->S_Set$S->{Methodbase}(TLorentzVector(o->x(),o->y(),o->z(),o->t())) );
    e->Send(*_m);
    $S->{Methodbase}_Update(o);
  } else {
    mIdol->Set$S->{Methodbase}(TLorentzVector(o->x(),o->y(),o->z(),o->t()));
  }
}\n
fnord
}

sub make_weed_update {
  my $S = shift;
  $S->make_weed_update_A() .
    "  const TLorentzVector& x = mIdol->Ref$S->{Methodbase}();\n" .
    "  w->set(x.T(),x.X(),x.Y(),x.Z());\n" .
  $S->make_weed_update_B();
}

########################################################################

1;
