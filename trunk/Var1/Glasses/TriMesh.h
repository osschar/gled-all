// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TriMesh_H
#define Var1_TriMesh_H

#include <Glasses/ZGlass.h>
#include <Stones/TringTvor.h>

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
  struct VConnData
  {
    Int_t   fVTarget;
    Float_t fDistance;
    Float_t fSpread;
    Float_t fSurface;
    // Eventually quantize these floats.
    Float_t fAngle;   // in some u-v coordinates from outside; sort criteria?
    Float_t fdU, fdV; // or something like this.
    // Somebody will surely add other stuff.

    VConnData(Int_t vtarget=0) :
      fVTarget(vtarget), fDistance(0), fSpread(0), fSurface(0) {}

    bool operator<(const VConnData &v) const { return fAngle < v.fAngle; }
  };

  struct VertexData
  {
    Float_t           fSpread;    // Sum of triangle angles
    Float_t           fSurface;   // Sum of 1/3 of triangle surfaces
    Int_t             fNeighbourConns;
    vector<VConnData> fVConns;

    VertexData() : fSpread(0), fSurface(0), fNeighbourConns(0), fVConns() {}

    VConnData& FindVConn(Int_t vtarget);
    void FindTwoVConnIdcs(Int_t v1, Int_t v2, Int_t& vi1, Int_t& vi2);
  };

private:
  void _init();

protected:
  ZLink<ZImage>           mDefTexture; //  X{GS} L{}

  TringTvor*              mTTvor;      //! X{gs}

  Opcode::Model*          mOPCModel;   //! X{g}
  Opcode::MeshInterface*  mOPCMeshIf;  //! X{g}

  vector<TriMesh::VertexData>   mVDataVec;  //! X{R}

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
  void CalculateBoundingBox();  // X{ED} 7 MButt()
  void GenerateVertexNormals(); // X{ED} 7 MButt()

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

#include "TriMesh.h7"
  ClassDef(TriMesh, 1)
}; // endclass TriMesh


#endif
