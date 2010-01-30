// $Id$

#include "RGBAPalette.h"
#include "RGBAPalette.c7"

#include <TColor.h>
#include <TMath.h>
#include <TROOT.h>
#include <TPRegexp.h>
#include <TStyle.h>
#include <TSystem.h>

#include <fstream>

//______________________________________________________________________
//
// Linear palette of RGBA colors for fast access via.
//
// Internal representation is std::vector<ZColorMark>. The float marks
// always go from 0 -> 1.
//
// This is expanded into mColorArray in steps from mMinInt to mMaxInt.
// ColorFromValue(Int_t val) makes direct access into this array.
// ColorFromValue(Float_t val) remaps the val

ClassImp(RGBAPalette);

void RGBAPalette::_init()
{
  mMinInt = 0; mMaxInt = 100;
  mMinFlt = 0; mMaxFlt = 1;

  bInterpolate     = true;    mShowDefValue    = true;
  mUnderflowAction = LA_Cut;  mOverflowAction  = LA_Clip;

  mDefaultColor.gray(0.5);    mCutColor.gray(0);
  mUnderColor.gray(0.9);      mOverColor.gray(0.1);

  mColorArray = 0;  mNBins = 0;
}

RGBAPalette::RGBAPalette(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

RGBAPalette::~RGBAPalette()
{
  delete [] mColorArray;
}

void RGBAPalette::ClearColorArray()
{
  if (mColorArray)
  {
    delete [] mColorArray;
    mColorArray = 0;
  }
}

/**************************************************************************/

void RGBAPalette::EmitRecolDCUORay()
{
  mDefaultColor.to_ubyte(mDefaultRGBA);
  mCutColor.to_ubyte(mCutRGBA);
  mUnderColor.to_ubyte(mUnderRGBA);
  mOverColor.to_ubyte(mOverRGBA);
}

/**************************************************************************/

void RGBAPalette::SetMinInt(Int_t min)
{
  mMinInt = min;
  if (mMaxInt <= min) mMaxInt = min + 1;
  ClearColorArray();
  Stamp(FID());
}

void RGBAPalette::SetMaxInt(Int_t max)
{
  mMaxInt = max;
  if (mMinInt >= max) mMinInt = max - 1;
  ClearColorArray();
  Stamp(FID());
}

/**************************************************************************/

void RGBAPalette::SetupColorArray() const
{
  if (mColorArray) delete [] mColorArray;

  mNBins = TMath::Max(mMaxInt-mMinInt+1, 1);
  mColorArray = new UChar_t [4 * mNBins];

  Float_t div = 1.0f / (mNBins - 1);

  UChar_t* p = mColorArray;
  mColorMarks.front().to_ubyte(p); p += 4;
  vector<ZColorMark>::const_iterator a = mColorMarks.begin(), b = a;
  for(Int_t i=1; i<mNBins - 1; ++i, p+=4)
  {
    Float_t f = i*div;
    while (b->mark() < f) a = b++;
    Float_t bfrac = (f - a->mark()) / (b->mark() - a->mark());
    if (bInterpolate)
    {
      ZColor c = (1 - bfrac)*(*a) + bfrac*(*b);
      c.to_ubyte(p);
    }
    else
    {
      if (bfrac <= 0.5f) a->to_ubyte(p);
      else               b->to_ubyte(p);
    }
  }
  mColorMarks.back().to_ubyte(p);
}

void RGBAPalette::SetMarksFromgStyle()
{
  static const Exc_t _eh("RGBAPalette::SetMarksFromgStyle ");

  Int_t nc = gStyle->GetNumberOfColors();
  if (nc < 2)
    throw _eh + "number of colors < 2.";
  mColorMarks.resize(nc);

  Float_t m   = 0;
  Float_t div = 1.0f / (nc - 1);
  UChar_t c[4];
  for (Int_t i=0; i<nc; ++i, m+=div)
  {
    ColorFromIdx(gStyle->GetColorPalette(i), c, true);
    mColorMarks[i].rgba_from_ubyte(c);
    mColorMarks[i].mark(m);
  }
  mColorMarks.back().mark(1);

  ClearColorArray();
  StampReqTring();
}

void RGBAPalette::SetMarksFromPOVFile(const Text_t* file_name)
{
  // Import gradient from POV like definition as exported by gimp.

  static const Exc_t _eh("RGBAPalette::SetMarksFromPOVFile ");

  ifstream ifs(file_name, ifstream::in);
  if (ifs.fail())
    throw _eh + "failed opening file '" + file_name + "'.";

  mColorMarks.clear();

  TPMERegexp line_re("^\\s*\\[.*\\]\\s*$");
  TPMERegexp num_re("[\\d\\.]+", "g");

  while (!ifs.eof())
  {
    TString text; text.ReadLine(ifs);

    if (line_re.Match(text))
    {
      Float_t n[5];
      Int_t   i = 0;
      num_re.ResetGlobalState();
      while (num_re.Match(text) && i < 5)
      {
	n[i++] = num_re[0].Atof();
      }
      if (i == 5)
      {
	mColorMarks.push_back(ZColorMark(n[0], n[1], n[2], n[3], 1.0f - n[4]));
      }
    }
  }
  printf("%s%zu marks from '%s'.\n", _eh.Data(), mColorMarks.size(), file_name);

  ifs.close();

  ClearColorArray();
  StampReqTring();
}

void RGBAPalette::PrintMarks() const
{
  static const Exc_t _eh("RGBAPalette::PrintMarks ");

  Int_t n = mColorMarks.size();
  printf("%s [%s], n_marks=%d\n", _eh.Data(), Identify().Data(), n);
  for (Int_t i=0; i<n; ++i)
  {
    const ZColorMark& m = mColorMarks[i];
    printf("%2d %5.3f  rgba:%5.3f/%5.3f/%5.3f/%5.3f\n",
           i, m.m(), m.r(), m.g(), m.b(), m.a());
  }
}

void RGBAPalette::PrintArray() const
{
  static const Exc_t _eh("RGBAPalette::PrintArray ");

  if (!mColorArray)  SetupColorArray();
  printf("%s [%s], n_bins=%d\n", _eh.Data(), Identify().Data(), mNBins);
  UChar_t* p = mColorArray;
  for (Int_t i=0; i<mNBins; ++i, p+=4)
  {
    printf("%3d rgba:%02hhx/%02hhx/%02hhx/%02hhx\n",
           i, p[0], p[1], p[2], p[3]);
  }
}

/**************************************************************************/
/**************************************************************************/

// Color from ROOT TColor

void RGBAPalette::ColorFromIdx(Short_t ci, UChar_t* col, Bool_t alpha)
{
  if (ci < 0)
  {
    col[0] = col[1] = col[2] = col[3] = 0;
    return;
  }
  TColor* c = gROOT->GetColor(ci);
  if(c)
  {
    col[0] = (UChar_t)(255*c->GetRed());
    col[1] = (UChar_t)(255*c->GetGreen());
    col[2] = (UChar_t)(255*c->GetBlue());
    if (alpha) col[3] = 255;
  }
}

void RGBAPalette::ColorFromIdx(Float_t f1, Short_t c1, Float_t f2, Short_t c2,
                               UChar_t* col, Bool_t alpha)
{
  TColor* t1 = gROOT->GetColor(c1);
  TColor* t2 = gROOT->GetColor(c2);
  if(t1 && t2)
  {
    col[0] = (UChar_t)(255*(f1*t1->GetRed()   + f2*t2->GetRed()));
    col[1] = (UChar_t)(255*(f1*t1->GetGreen() + f2*t2->GetGreen()));
    col[2] = (UChar_t)(255*(f1*t1->GetBlue()  + f2*t2->GetBlue()));
    if (alpha) col[3] = 255;
  }
}
