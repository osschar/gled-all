// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Tringula_H
#define Var1_Tringula_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/TimeMakerClient.h>

#include <TRandom.h>

class ZHashList;
class RectTerrain;
class TringTvor;
class TriMesh;
class Dynamico;

namespace Opcode {
class CollisionFaces;
class RayCollider;
class Plane;
class Point;
}


class Tringula : public ZNode, public TimeMakerClient
{
  MAC_RNR_FRIENDS(Tringula);

public:
  enum EdgeRule_e { ER_Stop, ER_Hold, ER_Bounce, ER_Wrap };

private:
  void _init();

  // Hack to carry intersection lines over to renderer.
  vector<Float_t> mItsLines;
  Int_t           mItsLinesIdx;

protected:
  ZLink<TriMesh> mMesh;         //  X{GS} L{}
  ZColor         mColor;        //  X{GSPT} 7 ColorButt()
  Bool_t         bPreferSmooth; //  X{GST}  7 Bool()

  ZLink<ZHashList> mDynos;      //  X{GS} L{}

  Opcode::CollisionFaces* mOPCCFaces;  //!
  Opcode::RayCollider*    mOPCRayCol;  //!

  Bool_t     bRnrRay; // X{GS} 7 Bool()
  TVector3   mRayPos; // X{GSR} 7 Vector3()
  Float_t    mRayL;   // X{GS} 7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Float_t    mRayT;   // X{GS} 7 Value(-range=>[-90,  90,  1,1000], -join=>1)
  Float_t    mRayP;   // X{GS} 7 Value(-range=>[-1000,1000,1,1000])

  Bool_t     bRnrDynos;  // X{GS} 7 Bool(-join=>1)
  Bool_t     bPickDynos; // X{GS} 7 Bool()
  Bool_t     bRnrBBoxes; // X{GS} 7 Bool(-join=>1)
  Bool_t     bRnrItsLines; // X{GS} 7 Bool() Intersection lines

  Int_t            mNumEPlanes; //!
  Opcode::Plane*   mEdgePlanes; //!
  EdgeRule_e       mEdgeRule;   //  X{GS} 7 PhonyEnum()

  TRandom          mRndGen;

  ZLink<TriMesh>   mDefDynMesh; // X{GS} L{}

  void get_ray_dir(Float_t* d, Float_t len=0);
  void handle_edge_crossing(Dynamico& D,
                            Opcode::Point& old_pos, Opcode::Point& pos,
                            Int_t plane, Float_t dist);

  void place_on_terrain(Dynamico& D);

public:
  Tringula(const Text_t* n="Tringula", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~Tringula();

  virtual void AdEnlightenment();

  void MakeOpcodeModel(); // X{ED} C{0} 7 MButt()
  void RayCollide();      // X{ED} C{0} 7 MButt()

  Dynamico* NewDynamico(const Text_t* dname=0);
  Dynamico* RandomDynamico(Float_t v_min=-1, Float_t v_max=10,
                           Float_t w_max= 1); // X{E} 7 MCWButt()

  void SetEdgePlanes(RectTerrain* rect_terr); // X{E} C{1} 7 MCWButt()

  void DoBoxPrunning(Bool_t detailed=false);  // X{E} 7 MCWButt()

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

#include "Tringula.h7"
  ClassDef(Tringula, 1);
}; // endclass Tringula


#endif
