// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TriMesh_H
#define Var1_TriMesh_H

#include <Glasses/ZGlass.h>

class ZImage;
class RectTerrain;
class TringTvor;
class GTSurf;

namespace Opcode {
class Model;
class MeshInterface;
}

class TriMesh : public ZGlass
{
  MAC_RNR_FRIENDS(TriMesh);

private:
  void _init();

protected:
  ZLink<ZImage>           mDefTexture; //  X{GS} L{}

  TringTvor*              mTTvor;      //! X{gs}

  Opcode::Model*          mOPCModel;   //! X{g}
  Opcode::MeshInterface*  mOPCMeshIf;  //! X{g}

public:
  TriMesh(const Text_t* n="TriMesh", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }
  virtual ~TriMesh();

  void BuildOpcStructs();

  // TringTvor interface
  void GenerateVertexNormals(); // X{ED} 7 MButt()

  void ImportRectTerrain(RectTerrain* rt); // X{ED} C{1} 7 MCWButt()

  void ImportGTSurf(GTSurf* gts);          // X{ED} C{1} 7 MCWButt()
  void ExportGTSurf(GTSurf* gts);          // X{ED} C{1} 7 MCWButt()

  void ExportPovMesh(const Text_t* fname, Bool_t smoothp=false); // X{E} 7 MCWButt()

  void MakeTetrahedron(Float_t l1=0.8, Float_t l2=0.2,
                       Float_t  w=0.4, Float_t  h=0.4); // X{E} 7 MCWButt()

#include "TriMesh.h7"
  ClassDef(TriMesh, 1)
}; // endclass TriMesh


#endif
