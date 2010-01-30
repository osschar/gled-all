// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Dynamico_H
#define Var1_Dynamico_H

#include <Glasses/Extendio.h>
#include <Stones/GravData.h>

class Statico;

class Dynamico : public Extendio
{
  friend class Tringula;
  MAC_RNR_FRIENDS(Dynamico);

private:
  void _init();

protected:
  HTransF         mLastTrans;  // Last transform from current master.
  Opcode::AABB    mLastAABB;   // Last enclosing bbox in parent frame.

  Float_t&        mV;          // X{GS}  7 Value(-range=>[-100,100, 1,1000], -join=>1)
  Float_t&        mW;          // X{GS}  7 Value(-range=>[  -5,  5, 1,1000])

  // Float_t         mPowPerM;    // X{GS}  7 Value(-range=>[-10,10, 1,1000])
  // Force per mass, perchance?
  // No ... force ... mass is eventually variable.
  // Then need "engine type"; min/max F, max dF/dt

  HPointF         mVVec;       // X{RS}  7 HPointF()
  HPointF         mWVec;       // X{RS}  7 HPointF()

  Float_t         mSafety;     //! Safe distance to tringula boundaries.
  Float_t         mExtraStep;  //! Extra distance traveled due collisions etc.

  GravData        mGrav;

  UInt_t          mOPCRCCache; //!

public:
  Dynamico(const Text_t* n="Dynamico", const Text_t* t=0);

  virtual void SetTringula(Tringula* tring);

  virtual void TimeTick(Double_t t, Double_t dt) {}

  virtual void update_last_data() { mLastTrans = mTrans; mLastAABB = mAABB; }

  virtual void update_grav_safeties(Float_t vl, Float_t vt);

  using Extendio::height_above_terrain;
  Float_t height_above_terrain() { return height_above_terrain(mGrav.Dir()); }

  static bool handle_collision(Dynamico            * dyno,
                               Statico             * stato,
                               Opcode::RayCollider & ray_collider,
                               Opcode::Ray         & ray,
                               Opcode::Point       & com_dyno,
                               CollisionSegments   & segments);

  static bool handle_collision(Dynamico            * dyno0,
                               Dynamico            * dyno1,
                               Opcode::Point       & up_dir,
                               CollisionSegments   & segments);

#include "Dynamico.h7"
  ClassDef(Dynamico, 1);
}; // endclass Dynamico

#endif
