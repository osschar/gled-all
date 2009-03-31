// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_FormulaMover_H
#define Geom1_FormulaMover_H

#include <Glasses/Operator.h>
#include <Stones/TimeMakerClient.h>

class TFormula;

class FormulaMover : public Operator, public TimeMakerClient
{
public:
  enum Operation_e { O_Off, O_Set, O_Add };

private:
  void _init();

protected:
  Operation_e   mPosMode; // X{GS} Ray{PosChanged} 7 PhonyEnum()
  TString       mX;       // X{GS} Ray{PosChanged} 7 Textor()
  TString       mY;       // X{GS} Ray{PosChanged} 7 Textor()
  TString       mZ;       // X{GS} Ray{PosChanged} 7 Textor()
  TFormula*     mForX;    // X{GS} Ray{PosChanged}
  TFormula*     mForY;    // X{GS} Ray{PosChanged}
  TFormula*     mForZ;    // X{GS} Ray{PosChanged}

  Operation_e   mRotMode; // X{GS} Ray{RotChanged} 7 PhonyEnum()
  TString       mPhi;     // X{GS} Ray{RotChanged} 7 Textor()
  TString       mTheta;   // X{GS} Ray{RotChanged} 7 Textor()
  TString       mEta;     // X{GS} Ray{RotChanged} 7 Textor()
  TFormula*     mForPhi;  // X{GS} Ray{RotChanged}
  TFormula*     mForTheta;// X{GS}
  TFormula*     mForEta;  // X{GS}

public:
  FormulaMover(const Text_t* n="FormulaMover", const Text_t* t=0) :
    Operator(n,t) { _init(); }

  void EmitPosChangedRay();
  void EmitRotChangedRay();

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

#include "FormulaMover.h7"
  ClassDef(FormulaMover, 1);
}; // endclass FormulaMover


#endif
