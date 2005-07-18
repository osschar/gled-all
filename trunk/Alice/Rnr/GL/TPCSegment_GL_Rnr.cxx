// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TPCSegment_GL_Rnr.h"
#include <Glasses/TPCSegRnrMod.h>
#include <FL/gl.h>

/**************************************************************************/

void TPCSegment_GL_Rnr::_init()
{
  // mSegRM = 0;
  mImage   = 0;
  mTexture = 0;
}

TPCSegment_GL_Rnr::~TPCSegment_GL_Rnr()
{
  if(mImage) delete mImage;
  if(mTexture) glDeleteTextures(1, &mTexture);
}
 
/**************************************************************************/

void TPCSegment_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mSegRMS);
  mSRM = (TPCSegRnrMod*) mSegRMS.fRnrMod->fLens;
  ZNode_GL_Rnr::Draw(rd);
}

void TPCSegment_GL_Rnr::Render(RnrDriver* rd)
{
  TPCDigitsInfo* info = mTPCSegment->mDigInfo;
  AliSimDigits *digit = &info->mSimDigits;
  AliTPCParam* par = info->mParameter;
  TTree* t = info->mTree;
  Int_t s,row,ent,off;

  if(mTPCSegment->mDigInfo == 0 ) {
    if (mSRM->bRnrFrame) display_frame(info);
    return;
  }
  // init mImage table
  if (mImage == 0 ){
    mImage = new GLubyte[ImageWidth*ImageHeight*4];
    glGenTextures (1,&mTexture);
  }
  memset(mImage, 0, ImageWidth*ImageHeight*4);

  ent = info->mSegEnt[mTPCSegment->mSegment];
  if(ent != -1) {
    row=0;
    //info->mInnSeg.dump();
    while(ent < t->GetEntriesFast()) {
      t->GetEntry(ent);
      par->AdjustSectorRow(digit->GetID(),s,row);
      // printf("AdjustSectorRow DigitID %d sector %d row %d \n",digit->GetID(),s,row );
      if(s != mTPCSegment->mSegment) break;
      off =  (info->mInnSeg.nMaxPads - par->GetNPadsLow(row))/2;
      load_padrow(row, off,0);
      ent++;
    }
  }
  ent = info->mSegEnt[mTPCSegment->mSegment + 36];
  if(ent != -1) {
    row=0;
    // info->mOut1Seg.dump();
    while(ent < t->GetEntriesFast()) {
      t->GetEntry(ent);
      par->AdjustSectorRow(digit->GetID(),s,row);
      // printf("AdjustSectorRow DigitID %d sector %d row %d \n",digit->GetID(),s,row );
      if(s != (mTPCSegment->mSegment+36)) break;

      if(row < par->GetNRowUp1()) {
	off =  (info->mOut1Seg.nMaxPads - par->GetNPadsUp(row))/2;
	load_padrow(row, off + info->mInnSeg.nMaxPads, 0);
      } else {
	off =  (info->mOut2Seg.nMaxPads - par->GetNPadsUp(row))/2;
	load_padrow(row, off, 0); // info->mInnSeg.nRows - info->mOut1Seg.nRows);
      }

      ent++;
    }
  }
  // rnr digits
  if(mSRM->bUseTexture) {
    init_texture(); 
   
    display_texture(info->mInnSeg.pad_width, info->mInnSeg.pad_length, info->mInnSeg.Rlow,
		    info->mInnSeg.nMaxPads, info->mInnSeg.nRows, 0,0);
    display_texture(info->mOut1Seg.pad_width, info->mOut1Seg.pad_length, info->mOut1Seg.Rlow,
		    info->mOut1Seg.nMaxPads,info->mOut1Seg.nRows,info->mInnSeg.nMaxPads,0);
    display_texture(info->mOut2Seg.pad_width, info->mOut2Seg.pad_length, info->mOut2Seg.Rlow,
		    info->mOut2Seg.nMaxPads, info->mOut2Seg.nRows,0,info->mOut1Seg.nRows);

    end_texture();
  }
  else { 
    display_quads(info->mInnSeg.pad_width, info->mInnSeg.pad_length, info->mInnSeg.Rlow,
		  info->mInnSeg.nMaxPads, info->mInnSeg.nRows, 0,0);
    display_quads(info->mOut1Seg.pad_width, info->mOut1Seg.pad_length, info->mOut1Seg.Rlow,
		  info->mOut1Seg.nMaxPads,info->mOut1Seg.nRows,info->mInnSeg.nMaxPads,0);
    display_quads(info->mOut2Seg.pad_width, info->mOut2Seg.pad_length, info->mOut2Seg.Rlow,
		  info->mOut2Seg.nMaxPads, info->mOut2Seg.nRows,0,info->mOut1Seg.nRows);
  }
  if(mSRM->bRnrFrame) display_frame(info);
}

/**************************************************************************/

void TPCSegment_GL_Rnr::load_padrow(Int_t row, Int_t col_off, Int_t row_off) 
{
  AliSimDigits *digit =  &mTPCSegment->mDigInfo->mSimDigits;

  Int_t    pad_var = 0;
  Int_t    time, pad, val;   
  GLubyte* img_pos;

  Int_t  min_time = mSRM->mTime - mSRM->mTimeWidth/2,
    max_time = mSRM->mTime + mSRM->mTimeWidth/2;
  Bool_t half_border_time = (mSRM->mTimeWidth % 2 == 0);

  Bool_t done_p = false;
  Bool_t save_p = false;

  if(mSRM->bUseLabels) digit->ExpandTrackBuffer();

  digit->First();
  do {

    time = digit->CurrentRow();
    pad  = digit->CurrentColumn();
    val  = digit->CurrentDigit();

    Bool_t use_digit = true;

    if(mSRM->bUseLabels) {
      use_digit = false;
      for(Int_t i=0; i<3; ++i) {
	Int_t l = digit->GetTrackIDFast(time, pad, i);
	if(l == 0) break;
	l -= 2;
	if(mSRM->HasLabel(l)) { use_digit = true; break; }
      }
    }

    if(use_digit) {
      if(mSRM->bShowMax) {
	if(val > pad_var) {
	  pad_var = val;
	}
      } else {
	// Integrate int max_val.
	if(time >= min_time && time <= max_time) {
	  if(half_border_time && (time == min_time || time == max_time))
	    pad_var += val/2;
	  else
	    pad_var += val;
	}
      }
    }

    if(digit->Next()) {
      if(pad != digit->CurrentColumn())
	save_p = true;
    } else {
      done_p = true;
      save_p = true;
    }

    if(save_p) {
      pad_var = TMath::Min(pad_var, mSRM->mMaxVal);
      if(pad_var > mSRM->mTreshold) {
	img_pos = get_row_col(row + row_off, pad + col_off);
	SetCol(pad_var, img_pos);
      }
      pad_var = 0; 
    }

  } while (!done_p);
}
 
/**************************************************************************/

void TPCSegment_GL_Rnr::SetCol(Float_t z, GLubyte* pixel)
{
  //  printf("set color row pad:  %d,  %d \n",row, cpad);
  Float_t c = (z - mSRM->mTreshold) * mSRM->mColSep /
    (mSRM->mMaxVal - mSRM->mTreshold);
  c -= (int)c;
  
  ZColor col;
  if(mSRM->mRibbon) {
    col =  mSRM->mRibbon->MarkToCol(c);
  } else {
    col = (1-c)*mSRM->mMinCol + c*mSRM->mMaxCol;
  }
  col.to_ubyte(pixel);
}

/**************************************************************************/

void TPCSegment_GL_Rnr::init_texture()
{
  glPushAttrib(GL_CURRENT_BIT      | GL_DEPTH_BUFFER_BIT |
	       GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT       | GL_POLYGON_BIT);
  glEnable(GL_BLEND);
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glShadeModel(GL_FLAT);

  glBindTexture  (GL_TEXTURE_2D, mTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexImage2D   (GL_TEXTURE_2D, 0, GL_RGBA,ImageWidth,               
		  ImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mImage);

  glPolygonOffset(2,2);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glEnable(GL_POLYGON_OFFSET_FILL);

  glEnable (GL_TEXTURE_2D);
  glTexEnvf (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
  glBindTexture (GL_TEXTURE_2D, mTexture);
}

void TPCSegment_GL_Rnr::display_texture (Float_t pw, Float_t pl, Float_t vR,
					 Int_t nMaxPads, Int_t nRows, 
					 Int_t startCol, Int_t startRow)
{
  Float_t w  = nMaxPads*pw/2;
  Float_t v1 = 1.0*startRow/ImageHeight;
  Float_t v2 = v1 + 1.0*nRows/ImageHeight;
  Float_t u1 = 1.0 *startCol/ImageWidth;
  Float_t u2 = u1 + 1.0 *nMaxPads/ImageWidth;
  // printf("tex coord u1,v1: (%f, %f), v2,u2: (%f,%f) \n", v1, u1, v2, u2);
  // printf("vertex coord >>> nPads %d pw %f, w: %f, y1: %f, y2: %f \n",nMaxPads,pw, w, vR, vR+nRows*pl);
  glColor4f(1.,1.,1.,mSRM->mAlpha); 
  glBegin (GL_QUADS);

  glTexCoord2f(u1, v1);     glVertex3f (-w,vR, 0.0);
  glTexCoord2f(u1, v2);     glVertex3f (-w,vR+nRows*pl,0.0);
  glTexCoord2f(u2, v2);     glVertex3f (w,vR+nRows*pl,0.0);
  glTexCoord2f(u2, v1);     glVertex3f (w,vR,0.0);

  /*
    glTexCoord2f(0., 0.);     glVertex3f (-w,vR, 0.0);
    glTexCoord2f(1., 0.);     glVertex3f (w,vR,0.0);
    glTexCoord2f(1, 1);     glVertex3f (w,vR+nRows*pl,0.0);
    glTexCoord2f(0., 1);     glVertex3f (-w,vR+nRows*pl,0.0);
  */

  glEnd();
}

void TPCSegment_GL_Rnr::end_texture()
{
  glPopAttrib();
}

/**************************************************************************/

void TPCSegment_GL_Rnr::display_quads (Float_t pw, Float_t pl, Float_t vR,
				       Int_t nMaxPads, Int_t nRows, 
				       Int_t startCol, Int_t startRow)
{
  GLubyte *pix;
  Float_t y_d, y_u;
  Float_t x_off, x;

  glBegin(GL_QUADS);
  for(Int_t row = 0; row<nRows; row++){
    y_d = vR + row*pl;
    y_u = y_d + pl;
    x_off = -nMaxPads*pw/2;
    for(Int_t pad = 0; pad<nMaxPads; pad++){
      pix = get_row_col(row + startRow, pad + startCol);
      x = x_off + pad*pw;
      if(pix[3] != 0){
	glColor4ubv(pix);
	glVertex3f(x+pw, y_d, 0);
	glVertex3f(x,    y_d, 0);
	glVertex3f(x,    y_u, 0);
	glVertex3f(x+pw, y_u, 0);
      }
    }
  }
  glEnd();
}

/**************************************************************************/

void TPCSegment_GL_Rnr::display_frame(TPCDigitsInfo* info)
{
  // AliTPCParam* par = info->mParameter;

  glColor4fv(mSRM->mFrameCol());
  TPCSeg* seg;

  seg = &info->mInnSeg;
  glBegin(GL_LINE_LOOP);
  loop_steps_up(seg);
  loop_steps_down(seg);
  glEnd();

  // outer1 segment
  seg = &info->mOut1Seg;
  glBegin(GL_LINE_LOOP);
  loop_steps_up(seg);
  loop_steps_down(seg);
  glEnd();

  // outer2 segment
  seg = &info->mOut2Seg;
  glBegin(GL_LINE_STRIP);
  loop_steps_up(seg);
  loop_steps_down(seg);
  glEnd();
}
