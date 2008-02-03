// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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

private:
  void _init();

protected:
  HTransF         mTrans;      // X{RPG} Transform from current master
  Opcode::AABB    mAABB;       //        Enclosing bbox in parent frame
  Bool_t          bAABBok;     // X{G}   Flag saying if mAABB is ok.

  Bool_t          bRnrSelf;    // X{GS}  7 Bool()

  ZLink<TriMesh>  mMesh;       // X{GS} L{}

public:
  Extendio(const Text_t* n="Extendio", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  const Opcode::AABB&     RefAABB()    { return ref_aabb(); }

  HTransF&                ref_trans()  { return mTrans; } // Use wisely.

  void                    touch_aabb() { bAABBok = false; }
  Opcode::AABB&           ref_aabb();

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
                                  vector<Opcode::Segment>& segments,
                                  const Text_t* debug_prefix=0);

#include "Extendio.h7"
  ClassDef(Extendio, 1)
}; // endclass Extendio


inline Opcode::AABB& Extendio::ref_aabb()
{
  if(!bAABBok) {
    mMesh->ref_opc_aabb().Rotate(mTrans, mAABB);
    bAABBok = true;
  }
  return mAABB;
}

#endif
