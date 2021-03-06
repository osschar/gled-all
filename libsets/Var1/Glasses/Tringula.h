// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Tringula_H
#define Var1_Tringula_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/TimeMakerClient.h>
#include <Eye/Ray.h>

#include <Opcode/Opcode.h>

#include <TRandom.h>

class ZHashList;
class ZVector;
class RectTerrain;
class TringTvor;
class TriMesh;
class ParaSurf;
class Extendio;
class Statico;
class Dynamico;
class Crawler;
class LandMark;
class Explosion;

class Tringula : public ZNode, public TimeMakerClient
{
  MAC_RNR_FRIENDS(Tringula);
  MAC_RNR_FRIENDS(TringuRep);
  friend class TringuCam;

public:
  enum PrivRayQN_e 
  {
    PRQN_offset = RayNS::RQN_user_0,
    PRQN_extendio_exploding, // followed by streamed ID of Extendio, Explosion
    PRQN_extendio_dying,     // followed by streamed ID of Extendio
    PRQN_extendio_sound      // followed by streamed ID of Extendio and TString naming the effect
  };

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

  struct PickResult
  {
    Extendio *fExtendio;
    Float_t   fTime;

    PickResult(Extendio* ext, Float_t t) : fExtendio(ext), fTime(t) {}

    bool operator<(const PickResult& o) { return fTime < o.fTime; }
  };

  typedef list<PickResult>           lPickResult_t;
  typedef list<PickResult>::iterator lPickResult_i;

private:
  void _init();

protected:
  ZLink<ParaSurf>    mParaSurf;     //  X{GS} L{aA}
  ZLink<TriMesh>     mMesh;         //  X{GS} L{aA}
  ZColor             mColor;        //  X{GSPRT} 7 ColorButt()
  Float_t            mMaxFlyerH;    //  X{GS}   7 Value(-const=>1,-join=>1)
  Float_t            mMaxCameraH;   //  X{GS}   7 Value()

  ZLink<ZHashList>   mStatos;       //  X{GS} L{} RnrBits{0,0,0,5}
  ZLink<ZHashList>   mDynos;        //  X{GS} L{} RnrBits{0,0,0,5}
  ZLink<ZHashList>   mFlyers;       //  X{GS} L{} RnrBits{0,0,0,5}
  ZLink<ZHashList>   mLandMarks;    //  X{GS} L{} RnrBits{0,0,0,5}
  ZLink<ZHashList>   mTubes;        //  X{GS} L{} RnrBits{0,0,0,5}

  ZLink<ZHashList>   mExplodios;    //  X{GS} L{} RnrBits{0,0,0,5}
  ZLink<ZHashList>   mExplosions;   //  X{GS} L{} RnrBits{0,0,0,5}

  GMutex             mExplosionMutex;             //!
  lpZGlass_t         mFreshExplodingExtendios;    //!
  lpZGlass_t         mFinishedExtendioExplosions; //!
  lpZGlass_t         mFreshExplosions;            //!
  lpZGlass_t         mFinishedExplosions;         //!

  Bool_t           bRnrBBoxes;   // X{GS} 7 Bool(-join=>1)

  EdgeRule_e       mEdgeRule;    //  X{GS} 7 PhonyEnum()

  TRandom          mRndGen;

  ZLink<TriMesh>   mDefStaMesh;  // X{GS} L{}
  ZLink<TriMesh>   mDefDynMesh;  // X{GS} L{}
  ZLink<TriMesh>   mDefFlyMesh;  // X{GS} L{}
  ZLink<TriMesh>   mDefChopMesh; // X{GS} L{}

  Opcode::BipartiteBoxPruner*  mBoxPruner;  //!
  // List-time-stamps for pruning sources.
  TimeStamp_t                  mStatosLTS;  //!
  TimeStamp_t                  mDynosLTS;   //!
  TimeStamp_t                  mFlyersLTS;  //!

  void handle_edge_crossing(Dynamico& D,
                            Opcode::Point& old_pos, Opcode::Point& pos,
                            Int_t plane, Float_t dist);

  Bool_t terrain_height(const Opcode::Point& pos, Float_t& point_h, Float_t& terrain_h);

  Bool_t place_on_terrain(Statico* S, TriMesh* M, Bool_t check_inside,
                          Float_t min_h_above=0);
  Bool_t place_on_terrain(Dynamico* D, Float_t h_above);

  void fill_pruning_list(AList* extendios, Int_t& n, const Opcode::AABB** boxes, void** user_data);
  void fill_pruning_list(AList* extendios, Int_t& n, Int_t l);
  void setup_box_pruner();
  void setup_stato_pruner();
  void setup_dyno_pruner();

  void prepick_extendios(AList* extendios, const Opcode::Ray& ray, Float_t ray_length,
                         lPickResult_t& candidates);

  void delete_lens_if_alive(ZGlass* lens);

public:
  Tringula(const Text_t* n="Tringula", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~Tringula();

  virtual void AdEnlightenment();

  // Collision stuff

  Bool_t    RayCollide(const Opcode::Ray& ray, Float_t ray_length,
		       Bool_t cull_p, Bool_t closest_p,
		       Opcode::CollisionFaces& col_faces);

  Float_t   RayCollideClosestHit(const Opcode::Ray& ray, Bool_t cull_p);

  Extendio* PickExtendios(const Opcode::Ray& ray, Float_t ray_length,
			  Float_t* ext_distance = 0);

  void      ResetCollisionStuff(); // X{ED} C{0} 7 MButt()

  Float_t   PlaceAboveTerrain(ZTrans& trans, Float_t height=0, Float_t dh_fac=0);

  Statico* NewStatico(const Text_t* sname=0);
  Statico* RandomStatico(ZVector* mesh_list,
                         Bool_t check_inside=true,
                         Int_t  max_tries   =1000);             // X{E} C{1} 7 MCWButt()

  void      RegisterCrawler(Crawler* d);
  Crawler*  NewDynamico(const Text_t* dname=0);
  Crawler*  RandomDynamico(ZVector* mesh_list,
                           Float_t v_min=-1, Float_t v_max=5,
                           Float_t w_max= 1);                      // X{E} C{1} 7 MCWButt()
  Dynamico* RandomAirplane(Float_t v_min= 2, Float_t v_max=20,
                           Float_t w_max= 1,
                           Float_t h_min_fac=0.3,
                           Float_t h_max_fac=1);                   // X{E} 7 MCWButt()
  Dynamico* RandomChopper (Float_t v_min= 2, Float_t v_max=20,
                           Float_t w_max= 1,
                           Float_t h_min_fac=0.1,
                           Float_t h_max_fac=0.5);                 // X{E} 7 MCWButt()

  LandMark* AddLandMark(TriMesh* mesh, const Float_t* pos);

  Bool_t CheckBoundaries(Dynamico* dyno, Float_t& safety);

  void DoFullBoxPrunning(vector<Opcode::Segment>& its_lines,
			 Bool_t accumulate=false, Bool_t verbose=false);

  void DoSplitBoxPrunning(); // X{E} 7 MCWButt()

  void ExtendioExploding(Extendio* ext);
  void ExtendioExplosionFinished(Explosion* exp);
  void ExplosionStarted(Explosion* exp);
  void ExplosionFinished(Explosion* exp);

  void LaserShot(Extendio* ext, const Opcode::Ray& ray, Float_t power);

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

  // Private Rays
  void EmitExtendioExplodingRay(Extendio* ext, Explosion* exp);
  void EmitExtendioDyingRay(Extendio* ext);
  void EmitExtendioSoundRay(Extendio* ext, const TString& effect);

#include "Tringula.h7"
  ClassDef(Tringula, 1);
}; // endclass Tringula

#endif
