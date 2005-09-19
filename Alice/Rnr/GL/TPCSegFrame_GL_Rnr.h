// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TPCSegFrame_GL_RNR_H
#define Alice_TPCSegFrame_GL_RNR_H

#include <Glasses/TPCSegFrame.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class TPCSegFrame_GL_Rnr : public ZNode_GL_Rnr {

 private:
  void _init();

 protected:
  TPCSegFrame*	mTPCSegFrame;
  Int_t         mCurrent;
 public:
  TPCSegFrame_GL_Rnr(TPCSegFrame* idol) :
    ZNode_GL_Rnr(idol), mTPCSegFrame(idol)
  { _init(); }

  virtual void Render(RnrDriver* rd); 
  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);
}; // endclass TPCSegFrame_GL_Rnr


#endif
