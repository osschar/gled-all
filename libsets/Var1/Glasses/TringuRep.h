// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuRep_H
#define Var1_TringuRep_H

#include <Glasses/ZNode.h>
#include "TriMesh.h"

class Tringula;
class TriMeshField;
class TriMeshLightField;
class RGBAPalette;

class TringuRep : public ZNode,
		  public TriMeshColorArraySource
{
  MAC_RNR_FRIENDS(TringuRep);

private:
  void _init();

protected:
  ZLink<Tringula>          mTringula;    // X{GE} L{a}

  ZLink<TriMeshField>      mField;       // X{GE} L{a}
  ZLink<TriMeshLightField> mLightField;  // X{GE} L{a}

  ZLink<RGBAPalette>       mPalette;     // X{GS} L{a}

  Bool_t                   bSmoothShading; // X{GE}  7 Bool()
  Bool_t                   bTringStrips;   // X{GST} 7 Bool()

  std::vector<UChar_t>     mVertCols;    // Vertex colors, 4*NVert of tringula's mesh
  std::vector<UChar_t>     mTringCols;   // Triangle colors, 4*NTrings of tringula's mesh

  // Should have color array, or texture (1d for field, 2d to also put
  // intensity from light-field as v coordinate).

  GMutex        mFieldMutex;    //! Lock for field access - terminal.

  Float_t       mActionValue;   // X{GS} 7 Value(-range=>[-100,  100, 1,100], -tooltip=>"value to add to field/source", -join=>1)
  Float_t       mActionRadius;  // X{GS} 7 Value(-range=>[   0,  100, 1,100], -tooltip=>"distance of vertices for which to add field/source")
  Float_t       mActionBorder;  // X{GS} 7 Value(-range=>[   0,   10, 1,100], -tooltip=>"extra border where value falls off linearly")

  Bool_t        bSprayDLWasOn;  //!

  void update_triangle_color_array();
  void refresh_color_arrays();

public:
  TringuRep(const Text_t* n="TringuRep", const Text_t* t=0);
  virtual ~TringuRep();

  // Virtuals from TriMeshColorArraySource.
  virtual void     AssertVertexColorArray();
  virtual UChar_t* GetVertexColorArray();
  virtual UChar_t* GetTriangleColorArray();
  virtual void     ColorArraysModified();

  // Link setters -- need to be somewhat special.
  void SetTringula  (Tringula*          tring);
  void SetField     (TriMeshField*      field);
  void SetLightField(TriMeshLightField* lightfield);

  void SetSmoothShading(Bool_t ss);
  void SwitchSmoothShading(); // X{E}

  // Real setters for field links.
  void ActivateField     (TriMeshField*      field);      // X{ED} C{1} 7 MCWButt()
  void ActivateLightField(TriMeshLightField* lightfield); // X{ED} C{1} 7 MCWButt()
  void SwitchLightField  (TriMeshLightField* lightfield); // X{ED} C{1} 7 MCWButt()

  void ColorByTerrainProps(Int_t mode=0); // X{ED} 7 MCWButt()

  void AddField(const Float_t point[3], Int_t vertex, Float_t factor);
  void BeginSprayField();
  void EndSprayField();

#include "TringuRep.h7"
  ClassDef(TringuRep, 1);
}; // endclass TringuRep

#endif
