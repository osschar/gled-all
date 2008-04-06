// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVSiteViz_H
#define AliEnViz_AEVSiteViz_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class AEVSiteViz : public ZNode {
  MAC_RNR_FRIENDS(AEVSiteViz);

private:
  void _init();

protected:
  ZColor     mColor;      // X{PGS} 7 ColorButt()

public:
  AEVSiteViz(const Text_t* n="AEVSiteViz", const Text_t* t=0) :
    ZNode(n,t) { _init(); }


#include "AEVSiteViz.h7"
  ClassDef(AEVSiteViz, 1)
}; // endclass AEVSiteViz


#endif
