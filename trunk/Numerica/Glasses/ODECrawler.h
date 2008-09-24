// $Header$

#ifndef Numerica_ODECrawler_H
#define Numerica_ODECrawler_H

#include <TVectorD.h>
#include <Glasses/ZGlass.h>

class ODECrawlerMaster
{
public:
  virtual ~ODECrawlerMaster() {}
  virtual UInt_t ODEOrder() = 0;
  virtual void ODEDerivatives(const Double_t x, const TVectorD& y, TVectorD& d) = 0;
  virtual void ODEStart(TVectorD& v, Double_t& x1, Double_t& x2) = 0;

  ClassDef(ODECrawlerMaster, 1);
};


//==============================================================================
// ODEStorage, ODEStorageT and typedefs
//==============================================================================

class ODEStorage
{
protected:
  Int_t      mOrder;
  Int_t      mSize;

public:
  ODEStorage() : 
    mOrder(0), mSize(0) {}
  ODEStorage(Int_t order, Int_t capacity=128) :
    mOrder(order), mSize(0) {}
  virtual ~ODEStorage() {}

  Int_t Order() const { return mOrder; }
  Int_t Size()  const { return mSize; }

  void ResetOrder(Int_t order, Int_t capacity=-1)
  {
    // Reset storage for new ODE-order and given capacity.
    // If capacity is -1 (default), 1.2 of size is used.

    mOrder = order;
    if (capacity == -1)
      capacity = 12*mSize/10;
    Reset(capacity);
  }

  void Reset()
  {
    // Reset storage. Capacity of containers is set to 1.2 of size.

    Reset(12*mSize/10);
  }

  virtual void Reset(Int_t capacity) = 0;

  virtual void AddEntry(Double_t x, Double_t* y) = 0;

  virtual Double_t GetMinXStored()   const = 0;
  virtual Double_t GetMaxXStored()   const = 0;
  virtual Double_t GetDeltaXStored() const { return GetMaxXStored() -  GetMinXStored(); }

  ClassDef(ODEStorage, 1);
};

template<typename TT>
class ODEStorageT : public ODEStorage
{
protected:
  vector<TT> mX;
  vector<TT> mY;

public:
  ODEStorageT(Int_t order, Int_t capacity=128) :
    ODEStorage(order, capacity)
  {
    mX.reserve(capacity);
    mY.reserve(mOrder*capacity);
  }
  virtual ~ODEStorageT() {}

  virtual void Reset(Int_t capacity)
  {
    // Reset storage to given capacity.

    vector<TT> x; x.reserve(capacity);        mX.swap(x);
    vector<TT> y; y.reserve(mOrder*capacity); mY.swap(y);
    mSize = 0;
  }

  virtual void AddEntry(Double_t x, Double_t* y)
  {
    mX.push_back(x);
    for (Int_t i = 0; i < mOrder; ++i)
      mY.push_back(y[i]);
    ++mSize;
  }

  virtual Double_t GetMinXStored()   const { return mSize > 0 ? mX[0]       : 0; }
  virtual Double_t GetMaxXStored()   const { return mSize > 0 ? mX[mSize-1] : 0; }
  virtual Double_t GetDeltaXStored() const { return mSize > 0 ? mX[mSize-1] - mX[0] : 0; }

  TT        GetX(Int_t i) const { return  mX[i];        }
  const TT* GetY(Int_t i) const { return &mY[mOrder*i]; }
  const TT* GetXArr()     const { return &mX[0];        }
  const TT* GetYArr()     const { return &mY[0];        }

  void      AssignY(Int_t i, TVectorT<TT>& v) const { v.Use(mOrder, &mY[mOrder*i]); }

  ClassDef(ODEStorageT, 1);
};

template class ODEStorageT<Float_t>;
template class ODEStorageT<Double_t>;

typedef ODEStorageT<Float_t>  ODEStorageF;
typedef ODEStorageT<Double_t> ODEStorageD;


//==============================================================================
// ODECrawler
//==============================================================================

class ODECrawler : public ZGlass
{
private:
  Double_t          hTINY, hSAFETY, hPGROW, hPSHRNK, hERRCON; //!
  ODECrawlerMaster *hTrueMaster; //!
  Bool_t            hCrawling;   //!

  void _init();

protected:
  ZLink<ZGlass> mODEMaster;     // X{GS} L{A}

  Int_t 	mGuessesOK;	//! X{g}  7 ValOut(-join=>1)
  Int_t 	mGuessesBad;	//! X{g}  7 ValOut()
  Int_t 	mStored;	//! X{g}  7 ValOut(-join=>1)
  Int_t 	mMaxSteps;	//  X{gS} 7 Value()
  Int_t 	mStoreMax;	//  X{gS} 7 Value(-join=>1)
  Double_t	mStoreDx;	//  X{gS} 7 Value()
  ODEStorage   *mStorage;       //  X{g}
  TVectorF*	mXStored;	//! X{g}
  TVectorF*	mYStored;	//! X{g} ROOT now allows streaming ... check syntax

  TVectorD	mY;		//! X{r}
  Int_t		mN;		//  X{g}  7 ValOut(-join=>1)
  Double_t	mAcc;		//  X{gS} 7 Value(-range=>[0,1])
  Double_t	mX1;		//  X{gS} 7 Value(-join=>1)
  Double_t	mX2;		//  X{gS} 7 Value()
  Double_t	mH1;		//  X{gS} 7 Value(-join=>1)
  Double_t	mHmin;		//  X{gS} 7 Value()

  void          init_integration(Bool_t call_ode_start);

  Int_t		Rkqs(TVectorD& y, TVectorD& dydx, Double_t& x, Double_t htry,
		     TVectorD& yscal, Double_t& hdid, Double_t& hnext);
  void 		Rkck(TVectorD& y, TVectorD& dydx, Double_t x, Double_t h,
		     TVectorD& yout, TVectorD& yerr);
  void		Integrate();

public:
  ODECrawler(const Text_t* n="ODECrawler", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  virtual ~ODECrawler();

  void        AdvanceXLimits(Double_t delta_x=0);

  void        SetStorage(ODEStorage* s);
  void        DetachStorage();
  ODEStorage* SwapStorage(ODEStorage* s);

  void        Crawl(Bool_t call_ode_start=true); // X{ED} 7 MCWButt()

#include "ODECrawler.h7"
  ClassDef(ODECrawler, 1);
}; // endclass ODECrawler


#endif
