// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZMatrix
#define Gled_ZMatrix

#include <Gled/GledTypes.h>
#include <Stones/ZVector.h>
#include <TMatrix.h>
class ZMatrixD;

class ZMatrix : public TMatrix {
public:
  ZMatrix() : TMatrix() {}
  ZMatrix(Int_t nrows, Int_t ncols) : TMatrix(nrows, ncols) {}
  ZMatrix(Int_t row_lwb, Int_t row_upb, Int_t col_lwb, Int_t col_upb) :
    TMatrix(row_lwb, row_upb, col_lwb, col_upb) {}
  ZMatrix(const ZMatrix& another) : TMatrix(another) {}
  ZMatrix(TMatrix::EMatrixCreatorsOp1 op, const ZMatrix& prototype) :
    TMatrix(op, prototype) {}
  ZMatrix(const ZMatrix& a, TMatrix::EMatrixCreatorsOp2 op, const ZMatrix& b) :
    TMatrix(a, op, b) {}
  ZMatrix(const ZMatrixD& x);
  ZMatrix(const TLazyMatrix& lazy_constructor) : TMatrix(lazy_constructor) {}

  // Unckecked Indexes
  Real_t& operator()(UCIndex_t rown, UCIndex_t coln) { return (fIndex[coln])[rown]; }
  Real_t operator()(UCIndex_t rown, UCIndex_t coln) const {
    return (fIndex[coln])[rown]; }
  Real_t& At(UCIndex_t rown, UCIndex_t coln) { return (fIndex[coln])[rown]; }
  Real_t At(UCIndex_t rown, UCIndex_t coln) const { return (fIndex[coln])[rown]; }
  // Checked access
  Real_t& operator()(Int_t rown, Int_t coln) {
    return TMatrix::operator()(rown,coln); }
  Real_t operator()(Int_t rown, Int_t coln) const {
    return TMatrix::operator()(rown,coln); }
  Real_t& At(Int_t rown, Int_t coln) {
    return TMatrix::operator()(rown,coln); }
  Real_t At(Int_t rown, Int_t coln) const {
    return TMatrix::operator()(rown,coln); }
  // Assignment ... TMatrix = copies object stuff, too
  ZMatrix& operator=(const ZMatrix &source);
  ZMatrix& operator=(const ZMatrixD &source);
  friend class ZMatrixD;
  ClassDef(ZMatrix, 1)
}; // endclass ZMatrix

inline ZMatrix &ZMatrix::operator=(const ZMatrix &source)
{
  if (this != &source && AreCompatible(*this, source)) {
    memcpy(fElements, source.fElements, fNelems*sizeof(Real_t));
  }
  return *this;
}

#endif
