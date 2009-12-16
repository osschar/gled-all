// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlass_GL_Rnr_H
#define GledCore_ZGlass_GL_Rnr_H

#include <Glasses/ZGlass.h>
#include <RnrBase/A_Rnr.h>
class RnrDriver;

#include <GL/glew.h>

class ZGlass_GL_Rnr : public A_Rnr
{
protected:
  ZGlass*	mGlass;
  TimeStamp_t	mStampTring;
  GLuint	mDispList;
  bool		bRebuildDL;

  bool          bUsesSubPicking;
  bool		bSuppressNameLabel;

  void obtain_rnrmod(RnrDriver* rd, RnrModStore& rms);
  bool check_gl_error(const TString& msg);

public:
  ZGlass_GL_Rnr(ZGlass* d=0);
  virtual ~ZGlass_GL_Rnr();

  // Virtuals from A_Rnr

  virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  virtual void PostDraw(RnrDriver* rd);

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

  // New virtuals

  virtual void Render(RnrDriver* rd);
  virtual void Triangulate(RnrDriver* rd) {}

  virtual void Redraw(RnrDriver* rd);

  //----------------------------------------------------------------------------

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

  class GL_Float_Holder
  {
    GLenum    fWhat;
    GLfloat   fState;
    bool      fFlip;
    void    (*fFoo)(GLfloat);

  public:
    GL_Float_Holder(GLenum what, GLfloat state, void (*foo)(GLfloat)) :
      fWhat(what), fFoo(foo)
    {
      glGetFloatv(fWhat, &fState);
      fFlip = (fState != state);
      if(fFlip) fFoo(state);
    }
    ~GL_Float_Holder()
    { if(fFlip) fFoo(fState); }
  };

}; // endclass ZGlass_GL_Rnr


#define GET_OR_RET(_t, _v, _p)         _t * _v = _p; if (!_v) return;
#define GET_OR_RET_VAL(_t, _v, _p, _r) _t * _v = _p; if (!_v) return _r;

#define REF_OR_RET(_t, _v, _p)         _t & _v = *_p; if (!&_v) return;
#define REF_OR_RET_VAL(_t, _v, _p, _r) _t & _v = *_p; if (!&_v) return _r;

#endif
