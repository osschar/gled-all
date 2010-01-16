// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuRep_GL_RNR_H
#define Var1_TringuRep_GL_RNR_H

#include <Glasses/TringuRep.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class Extendio;
class ExtendioExplosion;

class AlSource;

class TringuRep_GL_Rnr : public ZNode_GL_Rnr
{
protected:
  class TringulaSpy : public OptoStructs::A_View
  {
  protected:
    TringuRep_GL_Rnr *mMaster;

  public:
    TringulaSpy(OptoStructs::ZGlassImg* i, TringuRep_GL_Rnr* m);
    virtual ~TringulaSpy();

    virtual void AbsorbRay(Ray& ray);
  };

  TringuRep	*mTringuRep;
  TringulaSpy   *mTringulaSpy;

  typedef hash_multimap<Extendio*, AlSource*> hExt2AlSrc_t;
  typedef hExt2AlSrc_t::iterator              hExt2AlSrc_i;
  typedef pair<hExt2AlSrc_i, hExt2AlSrc_i>    hExt2AlSrc_ip;

  hExt2AlSrc_t mExtendioSounds;

public:
  TringuRep_GL_Rnr(TringuRep* idol);
  virtual ~TringuRep_GL_Rnr();

  virtual void SetImg(OptoStructs::ZGlassImg* newimg);

  //virtual void PreDraw(RnrDriver* rd);
  virtual void Draw(RnrDriver* rd);
  //virtual void PostDraw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);

  void WipeSpy() { mTringulaSpy = 0; }
  void ExtendioExploding(Extendio* ext, ExtendioExplosion* exp);
  void ExtendioDying(Extendio* ext);

}; // endclass TringuRep_GL_Rnr

#endif
