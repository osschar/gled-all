// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_ProductionRule_H
#define Tmp1_ProductionRule_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZNode.h>

class ProductionRule : public ZGlass
{
  MAC_RNR_FRIENDS(ProductionRule);

private:
  void _init();

protected:
  ZNode*  mConsumer; 
  TString	mRule;		//  X{RGE} 7 Textor()

public:
  ProductionRule(){assert(false);}
  ProductionRule(const Text_t*, const Text_t*);
  virtual ~ProductionRule();
  
  void SetRule(const Text_t* t);
  
  void SetConsumer(ZNode* n) {mConsumer = n;}
#include "ProductionRule.h7"
  ClassDef(ProductionRule, 1);
}; // endclass ProductionRule

#endif
