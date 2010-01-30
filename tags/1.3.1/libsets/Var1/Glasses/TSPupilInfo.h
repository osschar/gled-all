// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TSPupilInfo_H
#define Var1_TSPupilInfo_H

#include <Glasses/PupilInfo.h>
#include <Stones/TimeMakerClient.h>
#include <Stones/SGridStepper.h>

class ZDeque;
class ZHashList;
class ZVector;
class Scene;
class WGlWidget;

class TimeMaker;

class AlSource;

class Extendio;
class Spiritio;
class TringuRep;

class TSPupilInfo : public PupilInfo,
		    public TimeMakerClient
{
  MAC_RNR_FRIENDS(TSPupilInfo);

private:
  void _init();

protected:
  ZLink<TimeMaker>  mTimeMaker;       // X{GS} L{}

  ZLink<ZHashList>  mSelection;       // X{GS} L{}
  ZColor            mSelectionColor;  // X{GSPT} 7 ColorButt()

  ZLink<Spiritio>   mDefaultSpiritio; // X{GS} L{}
  ZLink<Spiritio>   mCurrentSpiritio; // X{GE} L{}
  Float_t           mZFovPreSpiritio; //

  // Menus / overlays.
  ZLink<Scene>      mMenuScene;       // X{GS} L{}
  ZLink<Scene>      mSpiritioScene;   // X{GS} L{}
  ZLink<WGlWidget>  mLastMenu;        // X{GS} L{}

  ZLink<TringuRep>  mTringuRep;       // X{GS} L{}

  SGridStepper      mGridStepper;     // X{r}

  // Collection of AlSources to be used by the renderers.
  // These variables are entirely managed by the renderers and are not
  // locked as rendering is done from a single thread.
  //
  // This should all go into a AlSourcePool glass in Audio1.
  // Static should be a parameter.
  static Int_t      sNMaxAlSources;
  Int_t             mNAlSources;      //!	
  ZLink<ZVector>    mAlSources;       //! X{GS} L{}
  ZLink<ZDeque>     mAlSourcesFree;   //!
  ZLink<ZHashList>  mAlSourcesUsed;   //!
  GMutex            mAlSourceMutex;   //!

public:
  TSPupilInfo(const Text_t* n="TSPupilInfo", const Text_t* t=0);
  virtual ~TSPupilInfo();

  virtual void AdEnlightenment();

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

  // Selection
  void SelectExtendio(Extendio* ext, Bool_t multiple); // X{E} C{1}

  // Overlay
  void AddOverlayElement(ZGlass* l);
  void RemoveOverlayElement(ZGlass* l);

  // Event handlers
  void AddEventHandler(ZGlass* l);
  void RemoveEventHandler(ZGlass* l);

  // Spiritios
  void SetCurrentSpiritio(Spiritio* s);
  void EnactExtendioSpiritio(Extendio* ext); // X{ED} C{1}

  // Menu handling.
  WGlWidget* FindMenuEntry(const TString& name);

  void SelectTopMenu(WGlWidget* weed);           // X{E} C{1} 7 MCWButt()
  void SelectTopMenuByName(const TString& name); // X{E}      7 MCWButt()
  void SelectTopMenuForLens(ZGlass* lens);       // X{E} C{1} 7 MCWButt()

  // Local AlSource management
  AlSource* AcquireAlSource();
  void      RelinquishAlSource(AlSource* src);

#include "TSPupilInfo.h7"
  ClassDef(TSPupilInfo, 1); // TringuSol PupilInfo.
}; // endclass TSPupilInfo

#endif
