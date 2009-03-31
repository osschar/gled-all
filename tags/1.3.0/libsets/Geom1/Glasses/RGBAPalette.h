// $Id$

#ifndef Geom1_RGBAPalette_H
#define Geom1_RGBAPalette_H

#include <Glasses/ZGlass.h>
#include <Stones/ZColorMark.h>

class RGBAPalette : public ZGlass
{
  MAC_RNR_FRIENDS(RGBAPalette);

public:
  enum LimitAction_e { LA_Cut, LA_Mark, LA_Clip, LA_Wrap };

private:
  void _init();

protected:
  Int_t     mMinInt;  // X{GET} 7 Value(-range=>[-1e6, 1e6, 1], -join=>1)
  Int_t     mMaxInt;  // X{GET} 7 Value(-range=>[-1e6, 1e6, 1])
  Float_t   mMinFlt;  // X{GST} 7 Value(-range=>[-1e6, 1e6, 1, 1000], -join=>1)
  Float_t   mMaxFlt;  // X{GST} 7 Value(-range=>[-1e6, 1e6, 1, 1000])

  Bool_t    bInterpolate;  // X{GST} R{ClearCA} 7 Bool(-join=>1)
  Bool_t    mShowDefValue; // X{GST} R{ClearCA} 7 Bool()

  LimitAction_e  mUnderflowAction; // X{GS} 7 PhonyEnum(-join=>1)
  LimitAction_e  mOverflowAction;  // X{GS} 7 PhonyEnum()

  ZColor    mDefaultColor;   // X{PGST} R{RecolDCUO} 7 ColorButt(-width=>8, -join=>1)
  ZColor    mCutColor;       // X{PGST} R{RecolDCUO} 7 ColorButt(-width=>8)
  ZColor    mUnderColor;     // X{PGST} R{RecolDCUO} 7 ColorButt(-width=>8, -join=>1)
  ZColor    mOverColor;      // X{PGST} R{RecolDCUO} 7 ColorButt(-width=>8)

  UChar_t   mDefaultRGBA[4]; //!
  UChar_t   mCutRGBA[4];     //!
  UChar_t   mUnderRGBA[4];   //!
  UChar_t   mOverRGBA[4];    //!

  mutable UChar_t* mColorArray; //[4*fNBins]
  mutable Int_t    mNBins;

  vector<ZColorMark> mColorMarks;

public:
  RGBAPalette(const Text_t* n="RGBAPalette", const Text_t* t=0);
  virtual ~RGBAPalette();

  void ClearColorArray();

  void EmitClearCARay() { ClearColorArray(); }
  void EmitRecolDCUORay();

  void SetMinInt(Int_t min);
  void SetMaxInt(Int_t max);

  // --------------------------------

  void SetupColorArray() const;

  void SetMarksFromgStyle();                         // X{E} 7 MCWButt(-join=>1)
  void SetMarksFromPOVFile(const Text_t* file_name); // X{E} 7 MCWButt()

  void PrintMarks() const; //! X{E} 7 MButt(-join=>1)
  void PrintArray() const; //! X{E} 7 MButt()

  // --------------------------------

  Bool_t   WithinVisibleRange(Int_t val) const;
  const UChar_t* ColorFromValue(Int_t val) const;
  const UChar_t* ColorFromValue(Int_t val, Int_t def_val) const;
  void     ColorFromValue(Int_t val, UChar_t* pix, Bool_t alpha=true) const;
  Bool_t   ColorFromValue(Int_t val, Int_t def_val, UChar_t* pix, Bool_t alpha=true) const;

  const UChar_t* ColorFromValue(Float_t val) const;
  void  ColorFromValue(Float_t val, UChar_t* pix, Bool_t alpha=true) const;

  // ================================================================

  static void ColorFromIdx(Short_t ci, UChar_t* col, Bool_t alpha=true);
  static void ColorFromIdx(Float_t f1, Short_t c1, Float_t f2, Short_t c2,
                           UChar_t* col, Bool_t alpha=true);

#include "RGBAPalette.h7"
  ClassDef(RGBAPalette, 1);
}; // endclass RGBAPalette


/**************************************************************************/
// Inlines for RGBAPalette
/**************************************************************************/

inline Bool_t RGBAPalette::WithinVisibleRange(Int_t val) const
{
  if ((val < mMinInt && mUnderflowAction == LA_Cut) ||
      (val > mMaxInt && mOverflowAction  == LA_Cut))
    return false;
  else
    return true;
}

inline const UChar_t* RGBAPalette::ColorFromValue(Int_t val) const
{
  // Here we expect that LA_Cut has been checked; we further check
  // for LA_Wrap and LA_Clip otherwise we proceed as for LA_Mark.

  if (!mColorArray)  SetupColorArray();
  if (val < mMinInt) {
    if (mUnderflowAction == LA_Wrap)
      val = (val+1-mMinInt)%mNBins + mMaxInt;
    else if (mUnderflowAction == LA_Clip)
      val = mMinInt;
    else
      return mUnderRGBA;
  }
  else if(val > mMaxInt) {
    if (mOverflowAction == LA_Wrap)
      val = (val-1-mMaxInt)%mNBins + mMinInt;
    else if (mOverflowAction == LA_Clip)
      val = mMaxInt;
    else
      return mOverRGBA;
  }
  return mColorArray + 4 * (val - mMinInt);
}

inline const UChar_t* RGBAPalette::ColorFromValue(Int_t val, Int_t def_val) const
{
  if (val == def_val && mShowDefValue)
    return mDefaultRGBA;
  else
    return ColorFromValue(val);
}

inline void RGBAPalette::ColorFromValue(Int_t val, UChar_t* pix, Bool_t alpha) const
{
  const UChar_t* c = ColorFromValue(val);
  pix[0] = c[0]; pix[1] = c[1]; pix[2] = c[2];
  if (alpha) pix[3] = c[3];
}

inline Bool_t RGBAPalette::ColorFromValue(Int_t val, Int_t def_val, UChar_t* pix, Bool_t alpha) const
{
  if (val == def_val) {
    if (mShowDefValue) {
      pix[0] = mDefaultRGBA[0];
      pix[1] = mDefaultRGBA[1];
      pix[2] = mDefaultRGBA[2];
      if (alpha) pix[3] = mDefaultRGBA[3];
      return true;
    } else {
      return false;
    }
  }

  if (WithinVisibleRange(val)) {
    ColorFromValue(val, pix, alpha);
    return true;
  } else {
    return false;
  }
}

inline const UChar_t* RGBAPalette::ColorFromValue(Float_t val) const
{
  Int_t vint = mMinInt +
    (Int_t) ((mMaxInt - mMinInt)*(val - mMinFlt)/(mMaxFlt - mMinFlt));
  return ColorFromValue(vint);
}

inline void  RGBAPalette::ColorFromValue(Float_t val, UChar_t* pix, Bool_t alpha) const
{
  const UChar_t* c = ColorFromValue(val);
  pix[0] = c[0]; pix[1] = c[1]; pix[2] = c[2];
  if (alpha) pix[3] = c[3];
}

#endif
