// $Header$
#ifndef Geom1_RectTerrain_GL_Rnr_H
#define Geom1_RectTerrain_GL_Rnr_H

#include <Glasses/RectTerrain.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class RectTerrain_GL_Rnr : public ZNode_GL_Rnr
{
protected:
  RectTerrain*		mTerrain;

  void MkBox(Int_t i, Int_t j);
  void MkVN(Int_t i, Int_t j);
  void MkCol(Float_t z);

  void check(double,double,GLdouble*,GLdouble*,GLint*);

  // Vars needed for coloring/texturing in explicit rendering modes.
  void setup_coltex_vars();
  Bool_t  bColP,  bTexP;
  Int_t   mMinX,  mMinY;
  Int_t   mMaxX,  mMaxY;
  Float_t mTexFx, mTexFy;
  Float_t mTexDx, mTexDy;

public:
  RectTerrain_GL_Rnr(RectTerrain* rt) :
    ZNode_GL_Rnr(rt), mTerrain(rt) {}

  virtual void Draw(RnrDriver* rd);

  virtual void Render(RnrDriver* rd);

}; // endclass RectTerrain_GL_Rnr

/**************************************************************************/

inline void RectTerrain_GL_Rnr::MkCol(Float_t z)
{
  if(mTerrain->mColSep) {
    Float_t c = (z - mTerrain->mMinZ) * mTerrain->mColSep /
	(mTerrain->mMaxZ - mTerrain->mMinZ);
    c -= (int)c;
    if(mTerrain->mRibbon != 0) {
      glColor4fv(mTerrain->mRibbon->MarkToCol(c)());
    } else {
      ZColor col( (1 - c)*mTerrain->mMinCol + c*mTerrain->mMaxCol );
      glColor4fv(col());
    }
  }
}

#endif
