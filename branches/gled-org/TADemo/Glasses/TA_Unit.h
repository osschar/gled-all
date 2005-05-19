// $Header$
#ifndef TADemo_TA_Unit_H
#define TADemo_TA_Unit_H

#include <Stones/ZColor.h>
#include <Glasses/ZNode.h>
#include "TA_SubUnit.h"
#include "TA_TextureContainer.h"

class TA_Unit : public TA_SubUnit {
  MAC_RNR_FRIENDS(TA_Unit);
  friend class TA_SubUnit;

  void _init();

protected:
  TString	       mFile;  	 // X{GS} 7 Filor()
  TA_TextureContainer* mTexCont; // X{gS} L{}

  ZColor	mColor;	  // X{PGS} 7 ColorButt()
  Float_t	mS;	  // X{GS} 7 Value(-range=>[0,100,1,100])
  Int_t		mJ;	  // X{GS} 7 Value(-range=>[0,3,1])

public:
  TA_Unit(Text_t* n="TA_Unit", Text_t* t=0) :
    TA_SubUnit(n,t) { _init(); }
  ~TA_Unit();

  void Load();	// X{E} 7 MButt()

#include "TA_Unit.h7"
  ClassDef(TA_Unit, 1)
}; // endclass TA_Unit

GlassIODef(TA_Unit);

#endif
