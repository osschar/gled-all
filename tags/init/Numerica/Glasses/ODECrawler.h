// $Header$

#ifndef Numerica_ODECrawler_H
#define Numerica_ODECrawler_H

#include <Stones/ZVector.h>
#include <Stones/ZVectorD.h>
#include <Glasses/Eventor.h>

class ODECrawlerMaster {
public:
  virtual ~ODECrawlerMaster() {}
  virtual UInt_t ODEOrder() = 0;
  virtual void ODEDerivatives(const Double_t x, const ZVectorD& y, ZVectorD& d) = 0;
  virtual void ODEStart(ZVectorD& v, Double_t& x1, Double_t& x2) = 0;

  ClassDef(ODECrawlerMaster,0)
};

class ODECrawler : public Eventor {

private:
  Double_t hTINY, hSAFETY, hPGROW, hPSHRNK, hERRCON;	//!
  ODECrawlerMaster*	hTrueMaster;			//!

  void _init();

protected:
  ZGlass*	mODEMaster;	// X{GS} L{}

  UInt_t	mGuessesOK;	//! X{G}  7 ValOut(-join=>1)
  UInt_t	mGuessesBad;	//! X{G}  7 ValOut()
  UInt_t	mStored;	//! X{G}  7 ValOut(-join=>1)
  UInt_t	mMaxSteps;	//  X{GS} 7 Value()
  UInt_t	mStoreMax;	//  X{GS} 7 Value(-join=>1)
  Double_t	mStoreDx;	//  X{GS} 7 Value()
  ZVector*	mXStored;	//! X{g}
  ZVector*	mYStored;	//! X{g} ROOT now allows streaming ... check syntax
  
  ZVectorD	mY;		//! X{r}
  UInt_t	mN;		// X{G}  7 ValOut(-join=>1)
  Double_t	mAcc;		// X{GS} 7 Value(-range=>[0,1])
  Double_t	mX1;		// X{GS} 7 Value(-join=>1)
  Double_t	mX2;		// X{GS} 7 Value()
  Double_t	mH1;		// X{GS} 7 Value(-join=>1)
  Double_t	mHmin;		// X{GS} 7 Value()
  
  Int_t		Rkqs(ZVectorD& y, ZVectorD& dydx, Double_t& x, Double_t htry,
		     ZVectorD& yscal, Double_t& hdid, Double_t& hnext);
  void 		Rkck(ZVectorD& y, ZVectorD& dydx, Double_t x, Double_t h,
		     ZVectorD& yout, ZVectorD& yerr);
  void		Crawl();

public:
  ODECrawler(Text_t* n="ODECrawler", Text_t* t=0) :
    Eventor(n,t), mODEMaster(0) { _init(); }
  ODECrawler(ZGlass* m, const Text_t* n="ODECrawler", const Text_t* t=0) :
    Eventor(n,t), mODEMaster(m) { _init(); }
  virtual ~ODECrawler();

  void	DetachStored() { mXStored = mYStored = 0; }

  // virtuals
  virtual Operator::Arg* PreDance(Operator::Arg* op_arg=0);
  virtual void Operate(Operator::Arg* op_arg) throw(Operator::Exception);


#include "ODECrawler.h7"
  ClassDef(ODECrawler, 1)
}; // endclass ODECrawler

GlassIODef(ODECrawler);

#endif
