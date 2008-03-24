// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Tringula_H
#define Var1_Tringula_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/TimeMakerClient.h>

#include <Opcode/Opcode.h>

#include <TRandom.h>

class ZHashList;
class ZVector;
class RectTerrain;
class RGBAPalette;
class TringTvor;
class TriMesh;
class ParaSurf;
class Extendio;
class Statico;
class Dynamico;


class Tringula : public ZNode, public TimeMakerClient
{
  MAC_RNR_FRIENDS(Tringula);
  friend class TringuCam;

public:
  enum EdgeRule_e { ER_Stop, ER_Hold, ER_Bounce, ER_Wrap };

  enum ColorSource_e
  {
    CS_XCoord, CS_YCoord, CS_ZCoord,
    // missing U,V,S Coord
    CS_XNorm,  CS_YNorm,  CS_ZNorm,
    // missing U,V,S Norm
    // fixed
    // external (defined by some function or vertex distribution)
    // shaded (ligt-source, ambient color/level),
  };

private:
  void _init();

  // Hack to carry intersection lines over to renderer.
  vector<Opcode::Segment>    mItsLines;    //!

protected:
  ZLink<ParaSurf>    mParaSurf;     //  X{GS} L{A}
  ZLink<TriMesh>     mMesh;         //  X{GS} L{A}
  ZLink<RGBAPalette> mPalette;      //  X{GS} L{A}
  ZColor             mColor;        //  X{GSPT} 7 ColorButt()
  Bool_t             bSmoothShade;  //  X{GST}  7 Bool(-join=>1)
  Bool_t             bLightMesh;    //  X{GST}  7 Bool()

  ZLink<ZHashList>   mStatos;       //  X{GS} L{}
  ZLink<ZHashList>   mDynos;        //  X{GS} L{}
  ZLink<ZHashList>   mFlyers;       //  X{GS} L{}

  Opcode::CollisionFaces* mRayColFaces;  //!

  Bool_t     bRnrRay; // X{GS}   7 Bool(-join=>1)
  Float_t    mRayLen; // X{GS}   7 Value(-range=>[-1000,1000,1,1000])
  TVector3   mRayPos; // X{GSRr} 7 Vector3()
  TVector3   mRayDir; // X{GSRr} 7 Vector3()

  Bool_t     bRnrDynos;  // X{GS} 7 Bool(-join=>1)
  Bool_t     bPickDynos; // X{GS} 7 Bool()
  Bool_t     bRnrBBoxes; // X{GS} 7 Bool(-join=>1)
  Bool_t     bRnrItsLines; // X{GS} 7 Bool() Intersection lines

  EdgeRule_e       mEdgeRule;   //  X{GS} 7 PhonyEnum()

  TRandom          mRndGen;

  ZLink<TriMesh>   mDefStaMesh; // X{GS} L{}
  ZLink<TriMesh>   mDefDynMesh; // X{GS} L{}
  ZLink<TriMesh>   mDefFlyMesh; // X{GS} L{}

  Opcode::BipartiteBoxPruner*  mBoxPruner; //!
  TimeStamp_t                  mStatosLTS;  //!
  TimeStamp_t                  mDynosLTS;   //!
  TimeStamp_t                  mFlyersLTS;  //!

  void get_ray_dir(Float_t* d, Float_t len=0);
  void handle_edge_crossing(Dynamico& D,
                            Opcode::Point& old_pos, Opcode::Point& pos,
                            Int_t plane, Float_t dist);

  Bool_t place_on_terrain(Statico* S, TriMesh* M, Bool_t check_inside,
                          Float_t min_h_above=0);
  Bool_t place_on_terrain(Dynamico* D);

  void   fill_pruning_list(AList* extendios, Int_t& n, const Opcode::AABB** boxes, void** user_data);
  void   fill_pruning_list(AList* extendios, Int_t& n, Int_t l);
  void   setup_box_pruner();
  void   setup_stato_pruner();
  void   setup_dyno_pruner();

public:
  Tringula(const Text_t* n="Tringula", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~Tringula();

  virtual void AdEnlightenment();

  // Color stuff

  void ColorByCoord(Int_t axis=2, Float_t fac=1, Float_t offset=0); // X{E} 7 MCWButt(-join=>1)
  void ColorByNormal(Int_t axis=2, Float_t min=-1, Float_t max=1);  // X{E} 7 MCWButt()

  void ColorByCoordFormula (const Text_t* formula="z",
                            Float_t min=0, Float_t max=10);         // X{E} 7 MCWButt(-join=>1)
  void ColorByNormalFormula(const Text_t* formula="sqrt(x*x+y*y)",
                            Float_t min=0, Float_t max=1);          // X{E} 7 MCWButt()

  // Collision stuff

  void SetRayVectors(const TVector3& pos, const TVector3& dir);

  void RayCollide();          // X{ED} C{0} 7 MButt()

  void ResetCollisionStuff(); // X{ED} C{0} 7 MButt()

  Float_t PlaceAboveTerrain(ZTrans& trans, Float_t height=0, Float_t dh_fac=0);

  Statico* NewStatico(const Text_t* sname=0);
  Statico* RandomStatico(ZVector* mesh_list,
                         Bool_t check_inside=true,
                         Int_t  max_tries   =1000);             // X{E} C{1} 7 MCWButt()

  Dynamico* NewDynamico(const Text_t* dname=0);
  Dynamico* RandomDynamico(ZVector* mesh_list,
                           Float_t v_min=-1, Float_t v_max=5,
                           Float_t w_max= 1);                   // X{E} C{1} 7 MCWButt()
  Dynamico* RandomFlyer   (Float_t v_min= 2, Float_t v_max=20,
                           Float_t w_max= 1, Float_t h_max=50); // X{E} 7 MCWButt()

  void SetEdgePlanes(RectTerrain* rect_terr); // X{E} C{1} 7 MCWButt()

  void DoFullBoxPrunning(Bool_t accumulate=false, Bool_t verbose=false); // X{E} 7 MCWButt()

  void DoSplitBoxPrunning(); // X{E} 7 MCWButt()

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

  void make_dyno_step(Dynamico* D, Float_t dt);

#include "Tringula.h7"
  ClassDef(Tringula, 1);
}; // endclass Tringula


#endif
