// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZGlass_GL_Rnr
#define Gled_ZGlass_GL_Rnr

#include <Glasses/ZGlass.h>
#include <RnrBase/A_Rnr.h>
class RnrDriver;

class ZGlass_GL_Rnr : public A_Rnr {
private:
protected:
  ZGlass*	mGlass;
  TimeStamp_t	mStampTring;

public:
  ZGlass_GL_Rnr(ZGlass* d=0) : A_Rnr(), mGlass(d), mStampTring(0) {}

  virtual ZGlass* GetGlass() { return mGlass; }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);
  // As an example of TimeStamp usage
  virtual void Triangulate();

  // GL attribute stack
  //virtual void PushGLAttrib() {}
  //virtual void PopGLAttrib() {}

}; // endclass ZGlass_GL_Rnr

#endif
