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

  bool          bUsesSubPicking;
  bool		bSuppressNameLabel;

  void obtain_rnrmod(RnrDriver* rd, RnrModStore& rms);

public:
  ZGlass_GL_Rnr(ZGlass* d=0);
  virtual ~ZGlass_GL_Rnr();

  virtual ZGlass* GetGlass() { return mGlass; }

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);
  virtual void Triangulate(RnrDriver* rd) {}

  virtual void Redraw(RnrDriver* rd);

  /**************************************************************************/

  class GL_Capability_Switch {
    GLenum    fWhat;
    GLboolean fState;
    bool      fFlip;

    void set_state(GLboolean s)
    { if(s) glEnable(fWhat); else glDisable(fWhat); }

  public:
    GL_Capability_Switch(GLenum what, GLboolean state) : fWhat(what)
    {
      fState = glIsEnabled(fWhat);
      fFlip  = (fState != state);
      if(fFlip)	set_state(state);
    }
    ~GL_Capability_Switch()
    { if(fFlip) set_state(fState); }
  };


}; // endclass ZGlass_GL_Rnr

#endif