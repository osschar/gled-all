// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVDemoDriver_H
#define AliEnViz_AEVDemoDriver_H

#include <Glasses/Text.h>
#include <TRandom.h>

class AEVMlClient;  class AEVDistAnRep;
class AEVMapViz;    class AEVJobRep;

class Eventor;      class TimeMaker;
class WGlDirectory; class WGlFrameStyle;
class WGlButton;

class Amphitheatre; class WSTube;

class AEVDemoDriver : public Text
{
  MAC_RNR_FRIENDS(AEVDemoDriver);

private:
  void _init();

protected:
  Double_t rnd(Double_t k=1, Double_t n=0);

  ZLink<AEVMlClient>    mMonaClient;	// X{GS} L{}
  ZLink<AEVDistAnRep>   mDistAnRep;	// X{GS} L{}

  ZLink<Amphitheatre>   mTheatre;	// X{GS} L{}
  ZLink<AEVMapViz>	mMapViz;	// X{GS} L{}
  ZLink<ZNode>          mGledViz;	// X{GS} L{}
  ZLink<ZNode>          mMonaViz;	// X{GS} L{}

  ZLink<Eventor>        mAnimator;	// X{GS} L{}
  ZLink<TimeMaker>      mAnimTimer;	// X{GS} L{}

  ZLink<WSTube>         mMonaTube;	// X{GS} L{}

  Float_t       mConnectTime;	// X{GS} 7 Value(-range=>[0,10,1,100], -join=>1)
  Float_t       mRotTime;	// X{GS} 7 Value(-range=>[0,10,1,100])
  Float_t       mTravelTime;	// X{GS} 7 Value(-range=>[0,10,1,100], -join=>1)
  Float_t       mWaitTime;      // X{GS} 7 Value(-range=>[0,10,1,100])

  Int_t		mFPS;   	// X{GS} 7 Value(-range=>[1,100,1]), -join=>1)
  Int_t		mRPS;   	// X{GS} 7 Value(-range=>[1,100,1]))

  ZLink<AList>          mTexCont;	 // X{GS} L{}
  ZLink<WGlDirectory>   mJobDirectory;	 // X{GS} L{}
  ZLink<WGlFrameStyle>  mMenuFrameStyle; // X{GS} L{}
  ZLink<Text>           mInfoBar;        // X{GS} L{}
  ZLink<Text>           mBlurpBar;       // X{GS} L{}

  TRandom	mRnd;		//!

  Bool_t        bWorking;       //!
  Bool_t        bCycling;	//!
  bool trylock();
  void unlock();
  void enable_menu();
  void disable_menu();

  // local stash
  //-------------
  map<string, WSTube*> mProcessTubes; //!

public:
  void sleep(Float_t s);
  void l_sleep(Float_t s);

  void set_text(const Text_t* txt);
  void set_blurp(const Text_t* txt);

  void setup_tube(WSTube* tube, const Text_t* texname="Photon");
  void animate_tube(WSTube* tube);
  void remove_tube(WSTube* tube);

  void rotate(ZNode* node, Float_t time, Int_t ai, Int_t aj);


public:
  AEVDemoDriver(const Text_t* n="AEVDemoDriver", const Text_t* t=0) :
    Text(n,t) { _init(); }

  void InitAnimationStuff(Eventor* e=0, TimeMaker* tm=0);

  void ResetDemo();	     // X{ED} 7 MButt(-join=>1)
  void RunDemo();  	     // X{ED} 7 MButt()

  void ConnectMonaLisa();    // X{Ed} 7 MButt(-join=>1)
  void DisconnectMonaLisa(); // X{Ed} 7 MButt()

  void QueryJobs();                         // X{Ed}      7 MButt()
  void QueryJobDetails(AEVJobRep* job_rep); // X{Ed} C{1} 7 MCWButt()

  void ReplayJob(WGlButton* button);        // X{Ed} C{1} 7 MCWButt()
  void CycleJob(WGlButton* button);         // X{Ed} C{1} 7 MCWButt()
  void DoJobReplay();                       // X{Ed}      7 MCWButt()
  void PauseJobReplay();                    // X{Ed}      7 MCWButt()
  void FinalizeJob();                       // X{Ed}      7 MCWButt()
  void ExitReplayMenu(WGlButton* button);   // X{Ed} C{1} 7 MCWButt()

  void ConnectToProof();     // X{Ed} 7 MButt()
  void ConnectProofSlaves(); // X{Ed} 7 MButt()
  void ProcessEvents();      // X{Ed} 7 MButt()

  void VisProofProgress(map<string,int>& siteevmap);

  void Report();             // X{Ed} 7 MButt()

protected:
  Bool_t bFakeProcessing;    // X{GS} 7 Bool()

public:
#include "AEVDemoDriver.h7"
  ClassDef(AEVDemoDriver, 1)
}; // endclass AEVDemoDriver


#endif
