// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TPCPadRow_GL_RNR_H
#define Alice_TPCPadRow_GL_RNR_H

#include <Glasses/TPCPadRow.h>
#include <Rnr/GL/TPCSegment_GL_Rnr.h>

class TPCPadRow_GL_Rnr : public ZNode_GL_Rnr {
 private:
  void _init();
  void  render_padrow(Int_t row);
  Int_t mNPads;

 protected:
  TPCPadRow*	       mTPCPadRow;  
  TPCSegRnrMod*	       mSRM;
  RnrModStore	       mSegRMS;
 public:
  TPCPadRow_GL_Rnr(TPCPadRow* idol) :
    ZNode_GL_Rnr(idol), mTPCPadRow(idol), mSegRMS(TPCSegRnrMod::FID())
  { _init(); }

  virtual void Draw(RnrDriver* rd);
  void Render(RnrDriver* rd);
  void SetColor(Float_t z);
  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

}; // endclass TPCPadRow_GL_Rnr

#endif
