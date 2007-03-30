// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVProofFeedback
//
//

#include "AEVProofFeedback.h"
#include <Glasses/AEVDistAnRep.h>
#include <Glasses/AEVDemoDriver.h>

#include <Glasses/ZImage.h>
#include <Glasses/Board.h>

#include <TList.h>
#include <TROOT.h>
#include <TProof.h>

#include <TError.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TH1.h>


ClassImp(AEVProofFeedback)

/**************************************************************************/

void AEVProofFeedback::_init()
{
  mProof = 0;
  mDAR   = 0;
  mDD    = 0;

  mHImg  = 0;
  mBoard = 0;
}

/**************************************************************************/

void AEVProofFeedback::Connect(TProof* p, AEVDistAnRep* dar)
{
  mProof = p;
  mDAR   = dar;
  //mProof->Connect("Feedback(TList *objs)", "AEVProofFeedback",
  //              this, "Feedback(TList *objs)");
  mProof->Connect("SiteProgress(TList *siteinfos)", "AEVProofFeedback",
                  this, "SiteProgress(TList *siteinfos)");

  mProof->Connect("Feedback(TList *objs)", "AEVProofFeedback",
		  this, "Feedback(TList *objs)");

  mFeedbackCount = 0;
}

void AEVProofFeedback::Disconnect()
{
  // if(mProof) mProof->Disconnect("Feedback(TList *objs)");
  if(mProof) {
    mProof->Disconnect("SiteProgress(TList *siteinfos)");
    mProof->Disconnect("Feedback(TList *objs)");
  }
  mProof = 0;
  mDAR   = 0;
}

/**************************************************************************/

void AEVProofFeedback::SiteProgress(TList *siteinfos)
{
  static const Exc_t _eh("AEVProofFeedback::SiteProgress ");

//   printf("%sgot a list w/ %d elements.\n", _eh.c_str(), siteinfos->GetSize());
//   TIter next_site(siteinfos);
//   TObject* o;
//   while( (o = next_site()) ) {
//     printf("  %s[%s]\n", o->GetName(), o->ClassName());
//   }
  
  // !!!! here create the map and pass it later to demo driver
  // !!!! to vizualize partial results coming back.

  map<string,int> siteevmap;
  if(mDAR) {
    mDAR->UpdateProcStatus(siteinfos, &siteevmap);
  }
  if(mDD) {
    mDD->VisProofProgress(siteevmap);
  }

}

void AEVProofFeedback::Feedback(TList *objs)
{
  static const Exc_t _eh("AEVProofFeedback::Feedback ");

  ++mFeedbackCount;

  TSeqCollection *canvases = gROOT->GetListOfCanvases();

  TVirtualPad *save = gPad;

  // ::Info("Feedback","DEREK: feedback called\n");

  TIter next(objs);
  TObject *o;
  // objs->Dump();
  while( (o = next()) )
    {
      if (o->InheritsFrom("TH1")) {

	TString name = o->GetName();
	// ::Info("Feedback","DEREK: feedback for %s\n",name.Data());
	name += "_canvas";

	TCanvas *c = (TCanvas*) canvases->FindObject( name.Data() );

	if ( c == 0 ) {
	  if(mHImg) {
	    c = new TCanvas(name.Data(),name.Data(), 1028, 540);
	  } else {
	    c = new TCanvas(name.Data(),name.Data(), 640, 480);	    
	  }
	  c->SetGrid(1, 1);
	  c->SetLogy(1);
	  ((TH1*)o)->DrawCopy();
	} else {
	  c->cd();
	  ((TH1*)o)->DrawCopy();
	}
	c->Update();

	// Update histo vizualization.
	if(mHImg) {
	  GLensWriteHolder wlck(mHImg);
	  // !!!! this is bugged. jpegs/pngs are slow, xpms don't work
	  // idealy would like a direct transfer.
	  // also ... segvs if the canvas is not completely visible.
	  c->SaveAs(mHImg->GetFile());
	  mHImg->Load();
	}
	// Updates come twice per second.
	if(mBoard && mFeedbackCount <= 100) {
	  GLensWriteHolder wlck(mBoard);
	  mBoard->SetColor(1,1,1, 0.01*mFeedbackCount);
	}
      }
    }

  if (save != 0) {
    save->cd();
  } else {
    gPad = 0;
  }
}

/**************************************************************************/
