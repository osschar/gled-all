// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZNodeLink_GL_RNR_H
#define GledCore_ZNodeLink_GL_RNR_H

#include <Glasses/ZNodeLink.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class ZNodeLink_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();
  void _setup_lens();
protected:
  ZNodeLink*	           mZNodeLink;
  OptoStructs::ZLinkDatum* mLensLD;

public:
  ZNodeLink_GL_Rnr(ZNodeLink* idol) :
    ZNode_GL_Rnr(idol), mZNodeLink(idol)
  { _init(); }

  virtual void SetImg(OptoStructs::ZGlassImg* newimg);

  virtual void CreateRnrScheme(RnrDriver* rd);

  virtual void PreDraw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

}; // endclass ZNodeLink_GL_Rnr

#endif
