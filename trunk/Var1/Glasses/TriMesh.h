// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TriMesh_H
#define Var1_TriMesh_H

#include <Glasses/ZGlass.h>
#include <Stones/TringTvor.h>

class ParaSurf;

class ZImage;
class RectTerrain;
class GTSurf;

namespace Opcode {
class Model;
class MeshInterface;
class AABB;
class Point;
}

class TriMesh : public ZGlass
{
  MAC_RNR_FRIENDS(TriMesh);

public:
  struct xx_edge
  {
    Int_t v1, v2;
    xx_edge(Int_t a, Int_t b) { if (a<b) v1=a, v2=b; else v1=b, v2=a; }

    // Symmetric operator== declared outside of TriMesh.

    struct hash 
    {
      size_t operator()(const xx_edge& xx) const
      { size_t i = (xx.v1 << 16) + xx.v2; return i; }

    };
  };

#ifndef __CINT__
  typedef hash_map<xx_edge, Int_t, xx_edge::hash> xx_edge_hash_t;
  typedef xx_edge_hash_t::iterator                xx_edge_hash_i;
#endif

  struct EdgeData
  {
    // Topology, depends on connectivity only
    Int_t   fV1, fV2;      // Vertex idcs
    Int_t   fT1, fT2;      // Triangle idcs on the left/right of v1->v2 (-1 for no tring)

    // Metrics & morphology, depends on vertex positions and parasurf
    Float_t fDistance;     // Distance between vertices
    Float_t fDh;           // Difference of height h(v2) - h(v1)
    Float_t fAngle;        // Angle in local f/g coords
    Float_t fSurface;      // Sum of 1/3 of triangle surfaces
    Float_t fSpr1, fSpr2;  // Spreads for vertices

    EdgeData();

    Bool_t is_first(Int_t v)     const { return v == fV1;  }
    Bool_t has_second_triangle() const  { return fT2 != -1; }

    Int_t other_vertex(Int_t v)   const { return v == fV1 ? fV2 : fV1; }
    Int_t left_triangle(Int_t v)  const { return v == fV1 ? fT1 : fT2; }
    Int_t right_triangle(Int_t v) const { return v == fV1 ? fT2 : fT1; }

    Float_t distance()      const { return fDistance; }
    Float_t dh(Int_t v)     const { return v == fV1 ? fDh : -fDh; }
    Float_t angle(Int_t v)  const
    { static const Float_t pi=3.14159265f; return v == fV1 ? fAngle : (fAngle >= pi ? fAngle - pi : fAngle + pi); }
    Float_t surface()       const { return fSurface; }
    Float_t spread(Int_t v) const { return v == fV1 ? fSpr1 : fSpr2; }

    Float_t sign(Int_t v)   const { return v == fV1 ? 1.0f : -1.0f; }

    Bool_t  match(Int_t va, Int_t vb) const
    { return (va == fV1 && vb == fV2) || (va == fV2 && vb == fV1); }
  };

  struct VertexData
  {
    Int_t             fNEdges;    // Number of edges
    Int_t            *fEdgeArr;   // Array of edge idcss, sorted by their angle
    Float_t           fSurface;   // Sum of 1/3 of triangle surfaces
    Float_t           fSpread;    // Sum of triangle angles

    VertexData() : fNEdges(0), fEdgeArr(0), fSurface(0), fSpread(0) {}

    Int_t n_edges()      const { return fNEdges; }
    Int_t edge(Int_t ei) const { return fEdgeArr[ei]; }

    // For use during construction
    void insert_edge(Int_t ei) { fEdgeArr[fNEdges++] = ei; }
  };

private:
  void _init();

protected:
  ZLink<ZImage>           mDefTexture; //  X{GS} L{}
  ZLink<ParaSurf>         mParaSurf;   //  X{GS} L{}

  TringTvor*              mTTvor;      //! X{gs}

  Opcode::Model*          mOPCModel;   //! X{g}
  Opcode::MeshInterface*  mOPCMeshIf;  //! X{g}

  vector<TriMesh::VertexData>   mVDataVec;  //! X{R}
  vector<TriMesh::EdgeData>     mEDataVec;  //! X{R}
  vector<Int_t>                 mECursVec;  //! X{R} Cursor array, one entry per vertex.

public:
  TriMesh(const Text_t* n="TriMesh", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }
  virtual ~TriMesh();

  virtual void ResetTTvorDependants();

  void StdSurfacePostImport();
  void StdDynamicoPostImport();

  Opcode::AABB& ref_opc_aabb() { return *(Opcode::AABB*)mTTvor->mCtrExtBox; }

  void BuildOpcStructs();
  void AssertOpcStructs();

  // TringTvor interface
  void CalculateBoundingBox();    // X{ED} 7 MButt()
  void GenerateVertexNormals();   // X{ED} 7 MButt()
  void GenerateTriangleNormals(); // X{ED} 7 MButt()
  void ImportRectTerrain(RectTerrain* rt,
                         Bool_t colp=true,
                         Bool_t texp=false); // X{ED} C{1} 7 MCWButt()

  void ImportGTSurf(GTSurf* gts); // X{ED} C{1} 7 MCWButt()
  void ExportGTSurf(GTSurf* gts); // X{ED} C{1} 7 MCWButt()

  void ExportPovMesh(const Text_t* fname, Bool_t smoothp=false); // X{E} 7 MCWButt()

  void MakeTetrahedron(Float_t l1=0.8, Float_t l2=0.2,
                       Float_t  w=0.4, Float_t  h=0.4); // X{E} 7 MCWButt()

  void BuildVertexConnections();
  void AssertVertexConnections();

  EdgeData& find_edge(const VertexData& vd, Int_t v1, Int_t v2)
  {
    for (Int_t i=0; i<vd.fNEdges; ++i)
    {
      EdgeData& ed = mEDataVec[vd.edge(i)];
      if (ed.is_first(v1) ? ed.fV2 == v2 : ed.fV1 == v2)
        return ed;
    }
    ZGlass* g=0; g->Print();
    throw(Exc_t("Safertundhell ... edge not found."));
  }

#include "TriMesh.h7"
  ClassDef(TriMesh, 1)
}; // endclass TriMesh


inline bool operator==(const TriMesh::xx_edge& a, const TriMesh::xx_edge& b)
{ return a.v1 == b.v1 && a.v2 == b.v2; }

#endif
