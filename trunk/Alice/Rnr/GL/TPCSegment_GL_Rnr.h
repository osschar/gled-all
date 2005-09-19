// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliDet_TPCSegment_GL_RNR_H
#define AliDet_TPCSegment_GL_RNR_H

#include <Glasses/TPCSegment.h>
#include <Glasses/TPCSegRnrMod.h>
#include <Stones/ZColor.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <RnrBase/RnrDriver.h>

class TPCSegRnrMod;
const int ImageWidth  = 256;
const int ImageHeight = 128;

class TPCSegment_GL_Rnr : public ZNode_GL_Rnr
{
 private:
 public:
  void          _init();
  void          load_padrow(Int_t row, Int_t off);
  void          display_texture(Float_t pw, Float_t pl, Float_t vR, Int_t nMaxPads, 
				Int_t nRows, Int_t startRow,Int_t startCol);
  void          display_quads(Float_t pw, Float_t pl, Float_t vR, Int_t nMaxPads, 
			      Int_t nRows, Int_t startRow,Int_t startCol);
  void          display_frame(TPCDigitsInfo* info);

  GLubyte* get_row(Int_t row);
  GLubyte* get_row_col(Int_t row, Int_t col);

  // protected:
  TPCSegment*	       mTPCSegment;
  GLubyte*             mImage;
  GLuint               mTexture;

  TPCSegRnrMod*	       mSRM;
  RnrModStore	       mSegRMS;
 
 public:
  TPCSegment_GL_Rnr(TPCSegment* idol) :
    ZNode_GL_Rnr(idol), mTPCSegment(idol), mSegRMS(TPCSegRnrMod::FID())
  { _init(); }
  virtual ~TPCSegment_GL_Rnr();

  virtual void Draw(RnrDriver* rd);
  virtual void Render(RnrDriver* rd);
  void         SetCol(Float_t z,GLubyte* pixel);
  void         InitTexture();
  void         EndTexture();

}; // endclass TPCSegment_GL_Rnr


inline  GLubyte* TPCSegment_GL_Rnr::get_row_col(Int_t row, Int_t col)
{
  if ( row > ImageHeight) printf("ERROR row %d ImageHeight %d\n", row, col);
  return mImage + (ImageWidth*row +col)*4*sizeof(GLubyte); //*sizeof();
}

inline void loop_steps_up(TPCSeg* seg){
  Float_t x = -(seg->nMaxPads*1.0/2 - seg->nsteps)*seg->pad_width;
  Float_t y  = seg->Rlow;
  glVertex3f(x,y,0.);
  for(int s = 0; s <seg->nsteps ;s++){
    y = seg->stepy[s];
    glVertex3f(x,y,0.);
    x -= seg->pad_width;
    glVertex3f(x,y,0.);
  }
  y =  seg->Rlow + seg->nRows*seg->pad_length;
  glVertex3f(-seg->nMaxPads*seg->pad_width/2,y,0.);
}

inline void loop_steps_down(TPCSeg* seg){
  Float_t x = seg->nMaxPads*seg->pad_width/2;
  Float_t y = seg->Rlow + seg->nRows*seg->pad_length;
  glVertex3f(x,y,0.);
  for (int s = (seg->nsteps -1); s >= 0 ;s--){
    y =  seg->stepy[s];
    glVertex3f(x,y,0.);
    x -= seg->pad_width;
    glVertex3f(x,y,0.);
  }
  y = seg->Rlow;
  glVertex3f((seg->nMaxPads*1.0/2 - seg->nsteps)*seg->pad_width,y, 0.);
}

#endif
