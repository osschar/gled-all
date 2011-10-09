// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_HTriMesh_H
#define Var1_HTriMesh_H

#include <Glasses/TriMesh.h>

class HTriMesh : public TriMesh
{
  MAC_RNR_FRIENDS(HTriMesh);

private:
  void _init();

public:

  struct HNode;
  //typedef vector<HNode>            vHNode_t;
  //typedef vector<HNode>::iterator  vHNode_i;
  typedef vector<HNode*>           vpHNode_t;
  typedef vector<HNode*>::iterator vpHNode_i;

  struct HLevel;
  typedef vector<HLevel>           vHLevel_t;
  typedef vector<HLevel>::iterator vHLevel_i;

  struct HNode
  {
    HPointF fHighestV;
    Int_t   fHighestVT;

    // Triangle representing this node in h-mesh
    Int_t   fT;           // = -1 for root of list

    // Child indexes
    Int_t   fFirstChildT; // = 0 for root, < 0 for leaf node
    Int_t   fNChildT;     // == 4 ? (or some other const stuff)
                          // hmh, this is 20 for initial state

    // Child nodes; empty for leaf nodes
    vpHNode_t fSubNodes;

    HNode() {}
    HNode(Int_t s, Int_t fc, Int_t nc) { Init(s, fc, nc); }

    ~HNode()
    {
      for (vpHNode_i i = fSubNodes.begin(); i != fSubNodes.end(); ++i) delete *i;
    }

    void Init(Int_t s, Int_t fc, Int_t nc)
    {
      fHighestV.Zero();
      fHighestVT = -1;
      fT = s;
      fFirstChildT = fc;
      fNChildT = nc;
    }

    Int_t FirstT() const { return fFirstChildT; }
    Int_t LastT()  const { return fFirstChildT + fNChildT - 1; }
  };

  struct HLevel
  {
    Int_t     fLevel;
    Int_t     fFirstT;
    Int_t     fNT;

    vpHNode_t fNodes;

    // Opcode model ...

    HLevel() {}
    HLevel(Int_t l, Int_t ft, Int_t nt, Int_t nn) { Init(l, ft, nt, nn); }

    void Init(Int_t l, Int_t ft, Int_t nt, Int_t nn)
    {
      fLevel = l; fFirstT = ft; fNT = nt;
      vpHNode_t v;
      v.reserve(nn);
      fNodes.swap(v);
    }

    Int_t FirstT() const { return fFirstT; }
    Int_t LastT()  const { return fFirstT + fNT - 1; }
  };

  struct TringTvorSubdivider
  {
#ifndef __CINT__
    TringTvor        &fTvor;
    TriMesh::hEdge_t  fEdgeMap;
#endif
    Int_t         fCurV;
    Int_t         fCurT;
    Int_t         fCurLevel;

    TringTvorSubdivider(TringTvor& t) : fTvor(t) {}

    void  BeginSubdivision(Int_t n_hierarhical, Int_t n_leaf);
    Int_t SubdivideEdge(Int_t v0, Int_t v1);
    Int_t SubdivideTriangle(Int_t t);
    void  EndSubdivision();
  };

protected:
  Int_t         mDrawLevel; // X{GE} 7 Value(-range=>[0, 100, 1])

  HNode         mRootNode; //!
  vHLevel_t     mLevels;   //!

  void subdivide_hierarhical(TringTvorSubdivider& tts);
  void subdivide_leaf       (TringTvorSubdivider& tts, Int_t n_leaf);

  void subdivide_leaf_rec(TringTvorSubdivider& tts, Int_t v0, Int_t v1, Int_t v2, Int_t depth);

public:
  HTriMesh(const Text_t* n="HTriMesh", const Text_t* t=0);
  virtual ~HTriMesh();

  void SetDrawLevel(Int_t l);
  void PrintLevels(); //! X{E} 7 MButt()

  void Subdivide(Int_t n_hierarhical, Int_t n_leaf);

#include "HTriMesh.h7"
  ClassDef(HTriMesh, 1);
}; // endclass HTriMesh

#endif
