// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVMlSucker_H
#define AliEnViz_AEVMlSucker_H

#include <Glasses/ZNameMap.h>
#include <Glasses/ZQueen.h>

#include <TPRegexp.h>

class AEVMlSucker : public ZNameMap
{
  MAC_RNR_FRIENDS(AEVMlSucker);

private:
  void _init();

protected:
  Int_t      mFooSleep;     // X{GS} 7 Value(-range=>[0,1e4,1])

  GThread   *mSuckerThread; //!

  TPMERegexp mGlobWordRE;   //!
  TPMERegexp mGlobVarRE;    //!
  TPMERegexp mUScoreRE;     //!

  TString next_word(const TString& s);
  bool    next_var (const TString& s, TString& var, TString& val);
  bool    next_var (const TString& s, TString& var, Double_t& val);

  static void* tl_Suck(AEVMlSucker* s);
  void Suck();

  template<class TT>
  TT* find_lens(const TString& path)
  {
    TT* lens = dynamic_cast<TT*>(mQueen->FindLensByPath(path));
    if (lens == 0)
      throw(Exc_t("Queen doesn't have '") + path + "'.");
    return lens;
  }

public:
  AEVMlSucker(const Text_t* n="AEVMlSucker", const Text_t* t=0) :
    ZNameMap(n,t) { _init(); }
  virtual ~AEVMlSucker() {}

  void StartSucker(); // X{E} 7 MButt()
  void StopSucker();  // X{E} 7 MButt()

#include "AEVMlSucker.h7"
  ClassDef(AEVMlSucker, 1);
}; // endclass AEVMlSucker


#endif
