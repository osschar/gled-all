// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Spiritio_GL_RNR_H
#define Var1_Spiritio_GL_RNR_H

#include <Glasses/Spiritio.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <Stones/SVars.h>

class AlSource;

class Spiritio_GL_Rnr : public ZNode_GL_Rnr
{
private:
  void _init();

protected:
  Spiritio*	mSpiritio;

  typedef map<Int_t, KeyHandling::KeyInfoRep> mK2KIRep_t;
  typedef mK2KIRep_t::iterator                mK2KIRep_i;

  mK2KIRep_t mKeyReps; //!

  void RegisterKey(Int_t k, const TString& tag);

  // ----------------------------------------------------------------

  void draw_vertical_outline_value_quads  (Float_t bar_hw, Float_t yzero, Float_t yval);
  void draw_horizontal_outline_value_quads(Float_t bar_hh, Float_t xzero, Float_t xval);

  void draw_vertical_picking_quad  (RnrDriver* rd, void* user_data, Float_t bar_hw);
  void draw_horizontal_picking_quad(RnrDriver* rd, void* user_data, Float_t bar_hh);

  void draw_vertical_minmaxvar  (const SMinMaxVarF& v, RnrDriver* rd, bool pick);
  void draw_horizontal_minmaxvar(const SMinMaxVarF& v, RnrDriver* rd, bool pick);

  void draw_vertical_desirevar  (const SDesireVarF& v, RnrDriver* rd, bool pick);
  void draw_horizontal_desirevar(const SDesireVarF& v, RnrDriver* rd, bool pick);

  void update_al_src(AlSource* src, const ZTrans& t, RnrDriver* rd);

public:
  Spiritio_GL_Rnr(Spiritio* idol);
  virtual ~Spiritio_GL_Rnr();

  virtual void AbsorbRay(Ray& ray);

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

  // New virtuals.
  virtual int  HandleKey  (RnrDriver* rd, Fl_Event& ev);
  virtual int  HandleMouse(RnrDriver* rd, Fl_Event& ev) { return 0; }

}; // endclass Spiritio_GL_Rnr

#endif
