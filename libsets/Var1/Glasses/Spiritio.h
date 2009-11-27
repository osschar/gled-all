// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Spiritio_H
#define Var1_Spiritio_H

#include <Glasses/ZNode.h>
#include <Stones/TimeMakerClient.h>
#include <Stones/KeyHandling.h>

class Camera;
class TSPupilInfo;

class Spiritio : public ZNode,
                 public TimeMakerClient
{
  MAC_RNR_FRIENDS(Spiritio);

private:
  void _init();

protected:
  // Do we need pupil-info? Or TSPupilInfo even.
  // Yes, if we want to modify global scene, overlay, ...
  // Or ... to get mouse ray vectors!
  // But heym if/when Camera is done right, the info should be
  // put there!
  ZLink<TSPupilInfo> mPupilInfo; // X{GS} L{}

  // link or virtual to get camera (Camera or ZNode?).
  ZLink<Camera>   mCamera; // X{GS} L{}

  Bool_t          mEatNonBoundKeyEvents; // X{GS} 7 Bool()

  typedef vector<KeyHandling::KeyInfo> vKeyInfo_t;
  typedef vKeyInfo_t::iterator         vKeyInfo_i;

  vKeyInfo_t mKeys; //!

  void  RegisterKey(KeyHandling::KeyInfo ki);
  Int_t FindKey(const TString& tag);

  static UInt_t sDoubleClickTime;

public:
  Spiritio(const Text_t* n="Spiritio", const Text_t* t=0);
  virtual ~Spiritio();

  virtual void Activate() {}
  virtual void Deactivate() {}

  virtual void HandleKey(Int_t key_idx, Bool_t downp, UInt_t time_elapsed); // X{E}

#include "Spiritio.h7"
  ClassDef(Spiritio, 1);
}; // endclass Spiritio

#endif
