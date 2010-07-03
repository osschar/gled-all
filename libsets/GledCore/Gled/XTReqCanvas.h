// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_XTReqCanvas_H
#define GledCore_XTReqCanvas_H

#include <Gled/TRootXTReq.h>

class TVirtualPad;
class TCanvas;

//------------------------------------------------------------------------------
// XTReqCanvas
//------------------------------------------------------------------------------

class XTReqCanvas : public TRootXTReq
{
  TString  fName, fTitle;
  Int_t    fW, fH, fNPx, fNPy;

  virtual void Act();

public:
  // Output
  TCanvas *fCanvas;

  XTReqCanvas(const char* name ="Gled Canvas",
	      const char* title="Default Gled Canvas",
	      int w=640, int h=480, int npx=1, int npy=1) :
    TRootXTReq("XTReqCanvas"),
    fName(name), fTitle(title),
    fW(w), fH(h), fNPx(npx), fNPy(npy),
    fCanvas(0)
  {}
  virtual ~XTReqCanvas() {}

  static TCanvas* Request(const char* name ="Gled Canvas",
			  const char* title="Default Gled Canvas",
			  int w=640, int h=480, int npx=1, int npy=1);
};


//------------------------------------------------------------------------------
// XTReqPadUpdate
//------------------------------------------------------------------------------

class XTReqPadUpdate : public TRootXTReq
{
  TVirtualPad *fPad;

  virtual void Act();

public:
  // Output

  XTReqPadUpdate(TVirtualPad* p) :
    TRootXTReq("XTReqPadUpdate"),
    fPad(p)
  {}
  virtual ~XTReqPadUpdate() {}

  static void Update(TVirtualPad* p);
};

#endif
