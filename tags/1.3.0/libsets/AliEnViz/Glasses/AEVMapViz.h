// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVMapViz_H
#define AliEnViz_AEVMapViz_H

#include <Glasses/ZNode.h>
#include <Glasses/ZRlNodeMarkup.h>

#include "AEVAlienUI.h"

class AEVSite;
class AEVSiteViz;
class ZImage;

class AEVMapViz : public ZNode
{
  MAC_RNR_FRIENDS(AEVMapViz);

private:
  void _init();

protected:
  ZLink<AList>         mSites;      // X{GS} L{}
  ZLink<ZRlNodeMarkup> mNameSwitch; // X{GS} L{}

public:
  AEVMapViz(const Text_t* n="AEVMapViz", const Text_t* t=0) : ZNode(n,t) { _init(); }
  virtual ~AEVMapViz() {}

  // Meant as initialization.
  void CutEarth_PDC04(ZImage* tex);       // X{E} C{1}

  Bool_t ImportSite(AEVSite* site, Bool_t warn=false); // X{E} C{1} 7 MCWButt()
  void   PopulateWSites(Bool_t warn=false); // X{E} 7 MButt()
  void   ClearSiteVizes();                  // X{E} 7 MButt()

  void RnrSiteNamesOn();  // X{E} 7 MButt(-join=>1)
  void RnrSiteNamesOff(); // X{E} 7 MButt()

  AEVSiteViz* FindSiteViz(const Text_t* name);

#include "AEVMapViz.h7"
  ClassDef(AEVMapViz, 1);
}; // endclass AEVMapViz


#endif
