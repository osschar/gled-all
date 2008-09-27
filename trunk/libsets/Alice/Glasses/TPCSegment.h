// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliDet_TPCSegment_H
#define AliDet_TPCSegment_H

#include <Glasses/ZNode.h>
#include <Stones/TPCDigitsInfo.h>

#include <TTree.h> 

// #include <AliTPCParam.h>
// #include <AliSimDigits.h>

class TPCSegment : public ZNode
{
  MAC_RNR_FRIENDS(TPCSegment);

 private:
  void _init();

 protected:
  TPCDigitsInfo*      mDigInfo; // X{gE}  
  Int_t               mSegment; // X{gE}   7 Value(-range=>[0,36,1])

 public:
  TPCSegment(const Text_t* n="TPCSegment", const Text_t* t=0) : ZNode(n,t)
  { _init(); }
  virtual ~TPCSegment();

  void SetDigInfo(TPCDigitsInfo* diginfo);
  virtual void SetSegment(Int_t segment);

#include "TPCSegment.h7"
  ClassDef(TPCSegment, 1)
}; // endclass TPCSegment


#endif
