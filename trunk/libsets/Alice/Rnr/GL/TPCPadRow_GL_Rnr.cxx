// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TPCPadRow_GL_Rnr.h"
#include <Eye/Eye.h>
#include <GL/gl.h>
#include <Stones/ZMIR.h>
#include <RnrBase/Fl_Event_Enums.h>
#include <Rnr/GL/GLTextNS.h>

/**************************************************************************/

void TPCPadRow_GL_Rnr::_init()
{
  bUsesSubPicking = true;
  mNPads = 0;
}

void TPCPadRow_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mSegRMS);
  mSRM = (TPCSegRnrMod*) mSegRMS.fRnrMod->fLens;
  ZNode_GL_Rnr::Draw(rd);
}


void TPCPadRow_GL_Rnr::Render(RnrDriver* rd)
{
  Float_t x0,y0;
  x0 =  mTPCPadRow->mDx/2.;
  y0 =  mTPCPadRow->mDy/2.;
  if(mTPCPadRow->mSegment != 0) {
    TPCSegment_GL_Rnr* seg_rnr = 
      dynamic_cast<TPCSegment_GL_Rnr*>(rd->GetRnr(fImg->fEye->DemanglePtr(*mTPCPadRow->mSegment)));
    AliTPCParam* par = mTPCPadRow->mSegment->GetDigInfo()->mParameter;//get from segment
    Int_t row = mTPCPadRow->mRow;
    Int_t nMaxPads,start_row,start_col,end_col;
    Int_t sec = mTPCPadRow->mSegment->GetSegment();

    if(row < par->GetNRowLow()) {
      start_row = row;
      mNPads = par->GetNPadsLow(row);
      nMaxPads = par->GetNPadsLow( par->GetNRowLow() - 1);
      start_col = (nMaxPads-mNPads)/2;
    } else {
      sec += 36;
      row = mTPCPadRow->mRow - par->GetNRowLow();
      mNPads = par->GetNPadsUp(row);

      if (row < par->GetNRowUp1()){
	nMaxPads = par->GetNPadsUp(par->GetNRowUp1()-1);
	start_row = row;
	start_col = (nMaxPads-mNPads)/2 + par->GetNPadsLow( par->GetNRowLow() - 1);

      }else {
	nMaxPads = par->GetNPadsUp(par->GetNRowUp()-1);
        start_row = row;
	start_col = (nMaxPads-mNPads)/2;
      }
    }
    end_col = start_col + mNPads;
    
    Float_t v1 = start_row*1./ImageHeight;
    Float_t v2 = v1 + 1./ImageHeight;
    Float_t u1 = start_col*1./ImageWidth;
    Float_t u2 = end_col*1./ImageWidth;

    //printf("tex coord u1,v1: (%f, %f), v2,u2: (%f,%f) \n", v1, u1, v2, u2);
    seg_rnr->InitTexture();
    glBegin (GL_QUADS);
    glTexCoord2f(u1, v1);     glVertex3f(-x0,  -y0, 0.);
    glTexCoord2f(u1, v2);     glVertex3f(-x0 ,  y0, 0.);
    glTexCoord2f(u2, v2);     glVertex3f( x0 ,  y0, 0.);
    glTexCoord2f(u2, v1);     glVertex3f( x0 , -y0, 0.);
    glEnd();
    seg_rnr->EndTexture();

    Double_t x,y,z;    
    mTPCPadRow->GetTrans().GetPos(x,y,z);

    GLTextNS::RnrText(rd, GForm("Seg: %d Sec: %d Row: %d Pad: %d",mTPCPadRow->mSegment->GetSegment(), sec, row, mTPCPadRow->mPad),
		      Int_t((x - mTPCPadRow->GetDx()*0.5*mTPCPadRow->GetSx())*rd->GetWidth()/10.)+ mTPCPadRow->mTX,
		      Int_t((y - mTPCPadRow->GetDy()*0.5*mTPCPadRow->GetSy())*rd->GetHeight()/10.) + mTPCPadRow->mTY, 
		      mTPCPadRow->mTZ, mTPCPadRow->PtrFgCol(), mTPCPadRow->PtrBgCol());
  }

  //frame
  glBegin(GL_LINE_LOOP);
  glColor4f(1.,0.,1.,1.);
  glVertex3f(-x0,  -y0, 0.);
  glVertex3f(-x0 ,  y0, 0.);
  glVertex3f( x0 ,  y0, 0.);
  glVertex3f( x0 , -y0, 0.);
  glEnd();
}




int TPCPadRow_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  if(ev.fEvent == FL_PUSH && ev.fButton == FL_LEFT_MOUSE) {
    TPCPadRow& R = *mTPCPadRow;

    Double_t x,y,z;    
    R.RefTrans().GetPos(x,y,z);
    Double_t dx = ev.fX*10./rd->GetWidth() - x + R.GetDx()*R.GetSx()*0.5;
    Int_t pad = Int_t(dx/(R.GetDx()*R.GetSx()/mNPads));
    R.SetPad(pad);
    printf("selcted pad %d of %d pos %f ev.pos in pix %d rel %f) \n",
                   pad, mNPads,  x,  ev.fX,ev.fX*10./rd->GetWidth());

    // find entry in digits tree
    R.mDigits = 0;
    AliTPCParam* par = R.mSegment->GetDigInfo()->mParameter;
    Int_t ent, sec;
    sec = R.mSegment->GetSegment();
    if(R.mRow > par->GetNRowLow()) sec += 36;
    
    ent = R.mSegment->GetDigInfo()->mSegEnt[sec];
    if(ent != -1) {
      Int_t s,row=0;
      TTree* t = R.mSegment->GetDigInfo()->mTree;
      R.mDigits = &R.mSegment->GetDigInfo()->mSimDigits;
      while(ent < t->GetEntriesFast()) {
	t->GetEntry(ent);
	par->AdjustSectorRow(R.mDigits->GetID(),s,row);
	// printf("AdjustSectorRow DigitID %d sector %d row %d \n",digit->GetID(),s,row );
	if(s != sec) break;
        if(row == R.mRow ){
          break;
	}
	ent++;
      }
      R.PrintPad();
      return 1;
    }
  }
  return 0;
}






