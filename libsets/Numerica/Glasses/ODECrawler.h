// $Id$

#ifndef Numerica_ODECrawler_H
#define Numerica_ODECrawler_H

#include <TVectorD.h>
#include <Glasses/ZGlass.h>

#include <gsl/gsl_odeiv.h>


class ODECrawlerMaster
{
public:
  virtual ~ODECrawlerMaster() {}

  virtual UInt_t ODEOrder() = 0;
  virtual void   ODEStart(Double_t y[], Double_t& x1, Double_t& x2) = 0;
  virtual void   ODEDerivatives(Double_t x, const Double_t y[], Double_t d[]) = 0;
  virtual bool   ODEHasJacobian() const { return false; }
  virtual void   ODEJacobian(Double_t x, const Double_t y[], Double_t* dfdy, Double_t dfdt[]) {}

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

typedef ODEStorageT<Float_t>  ODEStorageF;
typedef ODEStorageT<Double_t> ODEStorageD;


//==============================================================================
// ODECrawler
//==============================================================================

class ODECrawler : public ZGlass
{
public:
  enum StepFunc_e
  {
    SF_RK2, SF_RK4, SF_RKF45, SF_RKCK, SF_RK8PD, SF_RK2Imp, SF_RK4Imp,
    SF_BSImp, SF_Gear1, SF_Gear2
  };

  static const gsl_odeiv_step_type* s_get_step_func(StepFunc_e sf);

private:
  ODECrawlerMaster  *m_true_master; //!
  Bool_t             m_crawling;    //!

  gsl_odeiv_system  *m_gsl_system;  //!
  gsl_odeiv_evolve  *m_gsl_evolve;  //!
  gsl_odeiv_control *m_gsl_control; //!
  gsl_odeiv_step    *m_gsl_step;    //!

  void _init();

  void _gsl_alloc();
  void _gsl_free();

protected:
  ZLink<ZGlass> mODEMaster;     // X{GS} L{a}

  Int_t 	mStepOK;	//! X{G}  7 ValOut(-join=>1)
  Int_t 	mStepChanged;	//! X{G}  7 ValOut()
  Int_t 	mStored;	//! X{G}  7 ValOut(-join=>1)
  Int_t 	mMaxSteps;	//  X{GS} 7 Value()
  Int_t 	mStoreMax;	//  X{GS} 7 Value(-join=>1)
  Double_t	mStoreDx;	//  X{GS} 7 Value(-range=>[0,1e12])
  ODEStorage   *mStorage;       //  X{g}

  TVectorD	mY;		//! X{r}
  Int_t		mN;		//  X{G}  7 ValOut()
  Double_t	mEpsAbs;	//  X{GS} 7 Value(-range=>[0,1000], -join=>1)
  Double_t	mEpsRel;	//  X{GS} 7 Value(-range=>[0,1])
  Double_t	mFacVal;	//  X{GS} 7 Value(-range=>[0,100], -join=>1)
  Double_t	mFacDer;	//  X{GS} 7 Value(-range=>[0,100])
  Double_t	mH1;		//  X{GS} 7 Value(-join=>1)
  Double_t	mHmin;		//  X{GS} 7 Value()
  Double_t      mH;             //  X{G}  7 ValOut(-join=>1)
  Bool_t        bStoreH;	//  X{GS} 7 Bool()
  StepFunc_e    mStepFunc;      //  X{GS} 7 PhonyEnum()
  StepFunc_e    mPrevStepFunc;  //!
  Double_t	mX1;		//  X{GS} 7 Value(-join=>1)
  Double_t	mX2;		//  X{GS} 7 Value()

  void          init_integration(Bool_t call_ode_start);

  void		Integrate();

public:
  ODECrawler(const Text_t* n="ODECrawler", const Text_t* t=0);
  virtual ~ODECrawler();

  void        AdvanceXLimits(Double_t delta_x=0);

  void        SetStorage(ODEStorage* s);
  void        DetachStorage();
  ODEStorage* SwapStorage(ODEStorage* s);

  void        Crawl(Bool_t call_ode_start=true); // X{ED} 7 MCWButt()

  // If you really (really) know what you're doing.
  void        ChangeOrderInPlace(Int_t order);
  Double_t*   RawYArray();

#include "ODECrawler.h7"
  ClassDef(ODECrawler, 1);
}; // endclass ODECrawler


#endif
