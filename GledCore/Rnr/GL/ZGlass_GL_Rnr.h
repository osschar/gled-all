// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlass_GL_Rnr_H
#define GledCore_ZGlass_GL_Rnr_H

#include <Glasses/ZGlass.h>
#include <RnrBase/A_Rnr.h>
class RnrDriver;

#include <GL/gl.h>

class ZGlass_GL_Rnr : public A_Rnr {
private:
protected:
  ZGlass*	mGlass;
  TimeStamp_t	mStampTring;
  GLuint	mDispList;
  bool		bRebuildDL;
  bool          bUseNameStack;
  bool		bSuppressNameLabel;

public:
  ZGlass_GL_Rnr(ZGlass* d=0);
  virtual ~ZGlass_GL_Rnr();

  virtual ZGlass* GetGlass() { return mGlass; }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);
  virtual void Triangulate(RnrDriver* rd) {}

}; // endclass ZGlass_GL_Rnr

#endif
