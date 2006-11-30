// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Dynamico_H
#define Var1_Dynamico_H

#include <Glasses/ZGlass.h>
#include <Glasses/TriMesh.h>
#include <Stones/HTrans.h>

#include <Opcode/Opcode.h>

class TringTvor;

class Dynamico : public ZGlass
{
  friend class Tringula;
  MAC_RNR_FRIENDS(Dynamico);

private:
  void _init();

protected:
  HTransF      mTrans;    // X{RPG} Transform from current master
  Opcode::AABB mAABB;     //        Enclosing bbox in parent frame
  Bool_t       bAABBok;   // X{G}   Flag saying if mAABB is ok.

  Float_t    mV;          // X{GS}  7 Value(-range=>[-100,100, 1,1000], -join=>1)
  Float_t    mW;          // X{GS}  7 Value(-range=>[  -5,  5, 1,1000])
  Float_t    mLevH;       // X{GS}  7 Value(-range=>[   0,  5, 1,1000])

  Bool_t     bRnrSelf;    // X{GS}  7 Bool()

  ZLink<TriMesh> mMesh;   // X{GS} L{}

  UInt_t     mOPCRCCache; //!

public:
  Dynamico(const Text_t* n="Dynamico", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  HTransF& ref_trans() { return mTrans; } // Use wisely.

  void          touch_aabb() { bAABBok = false; }
  Opcode::AABB& ref_aabb()
  { if(!bAABBok) { mMesh->ref_opc_aabb().Rotate(mTrans, mAABB); bAABBok = true; }
    return mAABB;
  }

  Opcode::Model*         get_opc_model()   { return mMesh->GetOPCModel(); }
  Opcode::MeshInterface* get_opc_mesh_if() { return mMesh->GetOPCMeshIf(); }


#include "Dynamico.h7"
  ClassDef(Dynamico, 1)
}; // endclass Dynamico


#endif
