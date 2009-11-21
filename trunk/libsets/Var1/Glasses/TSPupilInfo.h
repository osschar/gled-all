// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TSPupilInfo_H
#define Var1_TSPupilInfo_H

#include <Glasses/PupilInfo.h>

class Scene;
class WGlWidget;

class TimeMaker;

class Spiritio;

class TSPupilInfo : public PupilInfo
{
  MAC_RNR_FRIENDS(TSPupilInfo);

private:
  void _init();

protected:
  ZLink<TimeMaker>  mTimeMaker;     // X{GS} L{}

  ZLink<Scene>      mMenuScene;     // X{GS} L{}
  ZLink<Scene>      mSpiritioScene; // X{GS} L{}

  ZLink<WGlWidget>  mLastMenu;      // X{GS} L{}

public:
  TSPupilInfo(const Text_t* n="TSPupilInfo", const Text_t* t=0);
  virtual ~TSPupilInfo();

  virtual void AdEnlightenment();

  // Overlay
  void AddOverlayElement(ZGlass* l);
  void RemoveOverlayElement(ZGlass* l);

  // Event handlers
  void AddEventHandler(ZGlass* l);
  void RemoveEventHandler(ZGlass* l);

  // Spiritios
  void InstallSpiritio(Spiritio* s);
  void UninstallSpiritio(Spiritio* s);

  // Menu handling.
  WGlWidget* FindMenuEntry(const TString& name);

  void SelectTopMenu(WGlWidget* weed);           // X{E} C{1} 7 MCWButt()
  void SelectTopMenuByName(const TString& name); // X{E}      7 MCWButt()

#include "TSPupilInfo.h7"
  ClassDef(TSPupilInfo, 1); // TringuSol PupilInfo.
}; // endclass TSPupilInfo

#endif
