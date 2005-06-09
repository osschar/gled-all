// $Header$

#ifndef Numerica_WarmAmoeba_H
#define Numerica_WarmAmoeba_H

#include <Glasses/Eventor.h>
#include <TVectorF.h>

#include <TRandom.h>

class WarmAmoebaMaster {
public:
  virtual ~WarmAmoebaMaster() {}
  virtual Float_t	Foon(const TVectorF& x) = 0;
  virtual TVectorF*	InitialState(TRandom& rnd) = 0;		// Amoeba deletes it
  virtual TVectorF*	InitialPerturbations(TRandom& rnd) = 0; // Amoeba deletes it
  virtual void		SetState(const TVectorF& x) = 0;
  virtual ZMIR*		S_SetState(const TVectorF& x) = 0;

  ClassDef(WarmAmoebaMaster,1)
};

class WarmAmoeba : public Eventor {
private:
  WarmAmoebaMaster* hTrueMaster; //!

  TVectorF*	    m_P;	 //! center + simplex vertices
  TVectorF	    m_Y;	 //! values at center + simplex vertices
  TVectorF	    m_Psum;	 //!
  TVectorF	    m_PBest;	 //! Best parameters

  Float_t	    m_y_best;	 //!
  Int_t		    m_iter;	 //!
  UInt_t	    m_n;	 //!
  Float_t	    m_T;	 //!
  Float_t	    m_T0;	 //!

  void		_init();
  void		_calc_psum();
  void		_export_algo_values(Operator::Arg* op_arg, bool bestp=false);
  Float_t&	P(Int_t m, Int_t n) { return m_P[m](n); }

protected:
  ZGlass*	mWA_Master;	// X{gS} L{}
  TRandom	mRanGen;

  // Low level store

  // Algorithm parameters, exported part
  UInt_t	mN;	// dimensions	X{gS} 7 ValOut(-join=>1)
  UInt_t	mSeed;	// 		X{gS} 7 Value(-range=>[0,MAX_ID,1])

  Float_t	mFTol;	// Fractional tolerance	X{gS} 7 Value(-range=>[0,1],join=>1)

  // Set T to T_0 times TFactor:
  Float_t	mTFactor;	// X{gS} 7 Value(-range=>[0,100], join=>1)
  // T_n = T_0(1 - n/NumSteps)^alpha 
  Float_t	mAlpha;		// X{gS} 7 Value(-range=>[1e-2,100,1,100])

  UInt_t	mMovesPerT;	// X{gS} 7 Value(-range=>[0,1e6,1],-width=>6,-join=>1)
  UInt_t	mNumSteps;	// X{gS} 7 Value(-range=>[0,1e6,1],-width=>6)

  Float_t	mT0;	// X{gS} 7 ValOut(-width=>8, -join=>1)
  Float_t	mT;	// X{gS} 7 ValOut(-width=>8, -range=>[0,100])

  Float_t	mYBest;	// Best value reached	 X{gS} 7 ValOut(-join=>1)
  Float_t	mYLast;	// Value after last move X{gS} 7 ValOut()

  // protected methods

  void		InitZStuff();	// Should be called from derived class::PreDance()
  Float_t	Ooze(Int_t ihi, Float_t& yhi, Float_t fac);
  void 		WAMove();

public:
  WarmAmoeba(const Text_t* n="WarmAmoeba", const Text_t* t=0);
  WarmAmoeba(ZGlass* m, const Text_t* n="WarmAmoeba", const Text_t* t=0);
  virtual ~WarmAmoeba();

  virtual Operator::Arg* PreDance(Operator::Arg* op_arg=0);
  virtual void		 PostDance(Operator::Arg* op_arg);

  virtual void	PostBeat(Operator::Arg* op_arg)	 throw(Operator::Exception);

  virtual void Operate(Operator::Arg* op_arg) throw(Operator::Exception);

#include "WarmAmoeba.h7"
  ClassDef(WarmAmoeba, 1)
}; // endclass WarmAmoeba

GlassIODef(WarmAmoeba);

#endif
