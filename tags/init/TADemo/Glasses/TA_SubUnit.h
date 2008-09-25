// $Header$
#ifndef TADemo_TA_SubUnit_H
#define TADemo_TA_SubUnit_H

#include <Glasses/ZNode.h>
#include <vector>

class TA_Unit;
class TA_TextureContainer;

struct o3dInfo;
struct _TA_Texture;

//#ifndef __CINT__
struct _TA_Prim {
  string	fTextureName;
  UShort_t	fN;
  UShort_t	fIndices[4]; // !!! Expect no more
  float		fNormal[3];
  _TA_Texture*	fTATex;
  _TA_Prim(const char* name, long ni) : fTextureName(name?name:""), fN((UShort_t)ni), fTATex(0) {}
  void calc_normal(float*, float*, float*);
};
typedef vector<_TA_Prim>		v_TA_Prim_t;
typedef vector<_TA_Prim>::iterator	v_TA_Prim_i;
//#endif

class TA_SubUnit : public ZNode {
  MAC_RNR_FRIENDS(TA_SubUnit);

protected:
  TA_Unit*	mUnit;		//! X{gS}
  o3dInfo*	mInfo;		//!
  int		mNVert;		//!
  // Latest Gled does not produce S_ methods for !-commented members
  int		mNPrim;		// X{GS} 7 ValOut()
  Int_t		mNPBeg;		// X{GS} 7 Value(-range=>[0,100,1])
  int		mNPEnd;		// X{GS} 7 Value(-range=>[0,100,1])
  //#ifndef __CINT__
  v_TA_Prim_t	mPrims;		//!
  //#endif
  float*	pVert;		//!
  float		mXoff;		//!
  float		mYoff;		//!
  float		mZoff;		//!

  void build(TA_Unit* u, o3dInfo* i, char* data);

public:
  TA_SubUnit(Text_t* n="TA_SubUnit", Text_t* t=0) :
    ZNode(n,t), mNPBeg(0), mNPEnd(0) {}

#include "TA_SubUnit.h7"
  ClassDef(TA_SubUnit, 1)
}; // endclass TA_SubUnit

GlassIODef(TA_SubUnit);

#endif
