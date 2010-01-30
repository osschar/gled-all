// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TriMeshLightField_H
#define Var1_TriMeshLightField_H

#include <Glasses/TriMeshField.h>

#include <TVector3.h>

class ZNode;

class TriMeshLightField : public TriMeshField
{
  MAC_RNR_FRIENDS(TriMeshLightField);

private:
  void _init();

protected:
  Float_t    mAmbiLit;      // X{GS} 7 Value(-range=>[0,1, 1,1000])
  Float_t    mAmbiShadowed; // X{GS} 7 Value(-range=>[0,1, 1,1000])
  Float_t    mAmbiBackFace; // X{GS} 7 Value(-range=>[0,1, 1,1000])

  TVector3   mLampPos;      // X{GSR} 7 Vector3()
  Bool_t     bDirectional;  // X{GS}  7 Bool()

public:
  TriMeshLightField(const Text_t* n="TriMeshLightField", const Text_t* t=0) :
    TriMeshField(n,t) { _init(); }

  void ModulateColors(Bool_t regen_tring_cols=true);  // X{E} 7 MCWButt()
  void PartiallyModulateColors(set<Int_t> vertices, Bool_t regen_tring_cols=true);

  void CalculateLightField(); // X{E} 7 MButt()

  void SetupLampPos(ZNode* lamp, ZNode* mesh);                // X{E} C{2} 7 MCWButt()
  void SetupLampDir(ZNode* mesh, Float_t theta, Float_t phi); // X{E} C{1} 7 MCWButt()

#include "TriMeshLightField.h7"
  ClassDef(TriMeshLightField, 1); // TriMeshFiled with support for light-map tracing.
}; // endclass TriMeshLightField


#endif
