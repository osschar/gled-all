// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_CrawlerSpiritio_H
#define Var1_CrawlerSpiritio_H

#include <Glasses/ExtendioSpiritio.h>

#include <Stones/KeyHandling.h>

class Crawler;

class CrawlerSpiritio : public ExtendioSpiritio
{
  MAC_RNR_FRIENDS(CrawlerSpiritio);

private:
  void _init();

protected:
  Crawler* get_crawler();

  KeyHandling::KeyInfo mKeyIncThrottle; //!
  KeyHandling::KeyInfo mKeyDecThrottle; //!

  KeyHandling::KeyInfo mKeyLeftWheel;   //!
  KeyHandling::KeyInfo mKeyRightWheel;  //!

  ZLink<ZNode>   mCameraBase; // X{GS} L{}

public:
  CrawlerSpiritio(const Text_t* n="CrawlerSpiritio", const Text_t* t=0);
  virtual ~CrawlerSpiritio();

  virtual void AdEnlightenment();

  // Spiritio
  virtual void Activate();
  virtual void Deactivate();

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

  // ----------------------------------------------------------------

  void IncThrottle(Int_t, Bool_t downp, UInt_t time_elapsed);
  void DecThrottle(Int_t, Bool_t downp, UInt_t time_elapsed);
  void SetThrottle(Float_t t); // X{E}

  void LeftWheel(Int_t, Bool_t downp, UInt_t time_elapsed);
  void RightWheel(Int_t, Bool_t downp, UInt_t time_elapsed);
  void SetWheel(Float_t w);    // X{E}

#include "CrawlerSpiritio.h7"
  ClassDef(CrawlerSpiritio, 1);
}; // endclass CrawlerSpiritio

#endif
