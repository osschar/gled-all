
// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_KeyHandling_H
#define Var1_KeyHandling_H

#include <Rtypes.h>
#include <TString.h>

#include <Gled/GTime.h>

class ZGlass;

namespace KeyHandling
{
  class EventHandler;
  class KeyHandler;

  struct Key
  {
    Bool_t      fIsDown;
    KeyHandler *fHandler;
  };

  struct KeyHandler
  {
    virtual void KeyUp  (Key* key) = 0;
    virtual void KeyDown(Key* key) = 0;
  };

  struct EventHandler
  {
    // map<int, Key*> ...
    virtual void RegisterKey  (Key* key);
    virtual void UnregisterKey(Key* key);

    // void Handle();
  };

  // integer key index, filled during dynamico init.

  // spiritio gets key up / down, dt in real time on rnr side. maybe also event
  // state (modifier buttons) has its own down state, separate from Rnr, which
  // also has its own state and can be told not to send auto-repeat.

  // basic key info on spiritio side


  class AKeyCallback
  {
  public:
    virtual ~AKeyCallback() {}

    virtual void Invoke(ZGlass*, Int_t, Bool_t, UInt_t) = 0;
  };

  template <typename T>
  class KeyCallback : public AKeyCallback
  {
    typedef void (T::*Foo_t)(Int_t, Bool_t, UInt_t);

    Foo_t mFunction;

  public:
    KeyCallback(Foo_t foo) : mFunction(foo) {}
    virtual ~KeyCallback() {}

    virtual void Invoke(ZGlass* lens, Int_t key, Bool_t downp, UInt_t elapsed)
    {
      ((((T*)lens)->*mFunction)(key, downp, elapsed));
    }
  };


  class KeyInfo
  {
  public:
    TString         fKeyTag;    // Unique tag used in GL to build tag - index map.
    TString         fKeyDesc;   // Description to be displayed in key setup dialog.
    AKeyCallback   *fCallback;

    Int_t           fIndex;     // Index in "compressed" key/action list in Spiritio.

    UChar_t         fDownCount; // Number of activations (key, overlay button, ...).

    KeyInfo(const TString& tag, const TString& desc, AKeyCallback* foo);

    ~KeyInfo();

    ClassDefNV(KeyInfo, 1);
  };

  //----------------------------------------------------------------------------

  // key info in the renderer. in principle could have to stages ... generic
  // and specialized -- with or without the function pointers.

  // then, there must be a switch somewhwere to choose between direct calls
  // and MIRs.

  class KeyInfoRep
  {
  public:
    Int_t    fIndex;
    Bool_t   fIsDown;
    GTime    fLastTime;

    KeyInfoRep()          : fIndex(-1),  fIsDown(kFALSE), fLastTime() {}
    KeyInfoRep(Int_t idx) : fIndex(idx), fIsDown(kFALSE), fLastTime() {}
  };
  
}

#endif
