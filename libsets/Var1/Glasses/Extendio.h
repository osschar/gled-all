// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Extendio_H
#define Var1_Extendio_H

#include <Glasses/ZGlass.h>
#include <Glasses/TriMesh.h>
#include <Stones/HTrans.h>

#include <Opcode/Opcode.h>

class TringTvor;

class Extendio : public ZGlass
{
  friend class Tringula;
  MAC_RNR_FRIENDS(Extendio);

public:
  class CollisionSegments : public vector<Opcode::Segment>
  {
  private:
    Opcode::Point           mCenter;

  protected:
    void calculate_center();

  public:
    CollisionSegments() { InvalidateDerivedQuantities(); }

    void Clear() { clear(); InvalidateDerivedQuantities(); }

    // Call if derived quantities have been used and new segments are
    // added afterwards.
    void InvalidateDerivedQuantities() { mCenter.SetNotUsed(); }

    const Opcode::Point& RefCenter()
    { if (mCenter.IsNotUsed()) calculate_center(); return mCenter; }
  };

private:
  void _init();

protected:
  HTransF         mTrans;        //  Transform from current master
  Opcode::AABB    mAABB;         //! Enclosing bbox in parent frame
  HTransF*        mLastTransPtr; //!
  Opcode::AABB*   mLastAABBPtr;  //!

  Bool_t          bRnrSelf;        // X{GS}  7 Bool()
  Bool_t          bVerboseCollide; // X{GS}  7 Bool()

  Bool_t          bSelected;       // X{GS}  7 BoolOut()

  ZLink<TriMesh>  mMesh;       // X{GS} L{}

public:
  Extendio(const Text_t* n="Extendio", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  const HTransF&        RefTrans()     const { return  mTrans; }
  const HTransF&        RefLastTrans() const { return *mLastTransPtr; }
  const Opcode::AABB&   RefAABB()      const { return  mAABB; }
  const Opcode::AABB&   RefLastAABB()  const { return *mLastAABBPtr; }

  HTransF&        ref_trans()       { return  mTrans; }        // Use wisely.
  HTransF&        ref_last_trans()  { return *mLastTransPtr; } // Use wisely.
  Opcode::AABB&   ref_aabb()        { return  mAABB; }
  Opcode::AABB&   ref_last_aabb()   { return *mLastAABBPtr; }

  void            update_aabb()     { mMesh->ref_mesh_bbox().Rotate(mTrans, mAABB); }

  virtual void update_last_data() {}

  Opcode::Model*          get_opc_model()   { return mMesh->GetOPCModel();  }
  Opcode::MeshInterface*  get_opc_mesh_if() { return mMesh->GetOPCMeshIf(); }

  TringTvor*              get_tring_tvor()  { return mMesh->GetTTvor(); }

  // --- Triangle-triangle intersection ---

  static bool intersect_triangle(Extendio* ext0,  Extendio* ext1,
                                 Int_t    tidx0,  Int_t    tidx1,
                                 Opcode::Segment& segment,
                                 const Text_t* debug_prefix=0);

  static int  intersect_extendios(Extendio* ext0, Extendio* ext1,
                                  Opcode::AABBTreeCollider& collider,
                                  vector<Opcode::Segment> & segments,
                                  const Text_t* debug_prefix=0);

#include "Extendio.h7"
  ClassDef(Extendio, 1);
}; // endclass Extendio

#endif
