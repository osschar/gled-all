// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVProofFeedback_H
#define AliEnViz_AEVProofFeedback_H

#include <Gled/GledTypes.h>
class AEVDistAnRep;
class AEVDemoDriver;
class ZImage;
class Board;

#include <TObject.h>
#include <TQObject.h>

class TList;
class TProof;

class AEVProofFeedback : public TObject, public TQObject {

private:
  void _init();

protected:
  TProof*        mProof; //! X{GS}
  AEVDistAnRep*  mDAR;	 //! X{GS}
  AEVDemoDriver* mDD;    //! X{GS}

  ZImage*        mHImg;  //! X{GS}
  Board*	 mBoard; //! X{GS}

  Int_t		 mFeedbackCount;

public:
  AEVProofFeedback() : TObject() { _init(); }

  void Connect(TProof* p, AEVDistAnRep* dar);
  void Disconnect();

  void SiteProgress(TList *siteinfos);
  void Feedback(TList *objs);

#include "AEVProofFeedback.h7"
  ClassDef(AEVProofFeedback, 1);
}; // endclass AEVProofFeedback

#endif
