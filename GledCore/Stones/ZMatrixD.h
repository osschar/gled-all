// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZMatrixD
#define Gled_ZMatrixD

#include <Gled/GledTypes.h>
#include <Stones/ZVectorD.h>
#include <TMatrixD.h>
class ZMatrix;

class ZMatrixD : public TMatrixD {
public:
  ZMatrixD() : TMatrixD() {}
  ZMatrixD(Int_t nrows, Int_t ncols) : TMatrixD(nrows, ncols) {}
  ZMatrixD(Int_t row_lwb, Int_t row_upb, Int_t col_lwb, Int_t col_upb) :
    TMatrixD(row_lwb, row_upb, col_lwb, col_upb) {}
  ZMatrixD(const ZMatrixD& another) : TMatrixD(another) {}
  ZMatrixD(TMatrixD::EMatrixCreatorsOp1 op, const ZMatrixD& prototype) :
    TMatrixD(op, prototype) {}
  ZMatrixD(const ZMatrixD& a, TMatrixD::EMatrixCreatorsOp2 op, const ZMatrixD& b) :
    TMatrixD(a, op, b) {}
  ZMatrixD(const ZMatrix& x);
  ZMatrixD(const TLazyMatrixD& lazy_constructor) : TMatrixD(lazy_constructor) {}

  // Unckecked Indexes
  Double_t& operator()(UCIndex_t rown, UCIndex_t coln) { return (fIndex[coln])[rown]; }
  Double_t operator()(UCIndex_t rown, UCIndex_t coln) const {
    return (fIndex[coln])[rown]; }
  Double_t& At(UCIndex_t rown, UCIndex_t coln) { return (fIndex[coln])[rown]; }
  Double_t At(UCIndex_t rown, UCIndex_t coln) const { return (fIndex[coln])[rown]; }
  // Checked access
  Double_t& operator()(Int_t rown, Int_t coln) {
    return TMatrixD::operator()(rown,coln); }
  Double_t operator()(Int_t rown, Int_t coln) const {
    return TMatrixD::operator()(rown,coln); }
  Double_t& At(Int_t rown, Int_t coln) {
    return TMatrixD::operator()(rown,coln); }
  Double_t At(Int_t rown, Int_t coln) const {
    return TMatrixD::operator()(rown,coln); }
  // Assignment ... TMatrixD = copies object stuff, too
  ZMatrixD& operator=(const ZMatrixD &source);
  ZMatrixD& operator=(const ZMatrix &source);

  friend class ZMatrix;
  ClassDef(ZMatrixD, 1)
}; // endclass ZMatrixD

inline ZMatrixD &ZMatrixD::operator=(const ZMatrixD &source)
{
  if (this != &source && AreCompatible(*this, source)) {
    memcpy(fElements, source.fElements, fNelems*sizeof(Double_t));
  }
  return *this;
}

#endif
