// $Header$

#include "WarmAmoeba.h"

#include <Ephra/Mountain.h>
#include <Gled/GledMirDefs.h>

#include <memory>

ClassImp(WarmAmoeba)

/**************************************************************************/
// private

void WarmAmoeba::_init()
{
  hTrueMaster = 0; m_P = 0;

  mSeed = 0;
  mFTol = 1e-12;
  mTFactor = 3; mAlpha = 1.5;
  mMovesPerT = 1000; mNumSteps = 100;
  mT0 = mT = 0;
  mYBest = mYLast = 0;
}

void WarmAmoeba::_calc_psum()
{
  for(UCIndex_t i=0; i<m_n; i++) {
    m_Psum(i) = 0;
    for(UCIndex_t j=0; j<=m_n; j++) m_Psum(i) += P(j,i);
  }
}

void WarmAmoeba::_export_algo_values(Operator::Arg* op_arg, bool bestp)
{
  OP_EXE_OR_SP_MIR(this, SetT, m_T);
  OP_EXE_OR_SP_MIR(this, SetYLast, m_Y(0u));
  OP_EXE_OR_SP_MIR(this, SetYBest, m_y_best);
  if(bestp) {
    OP_EXE_OR_SP_MIR_SATURN(mSaturn, hTrueMaster, SetState, m_PBest);
  } else {
    OP_EXE_OR_SP_MIR_SATURN(mSaturn, hTrueMaster, SetState, m_P[0]);    
  }
}

/**************************************************************************/
// protected

void WarmAmoeba::InitZStuff()
{
  m_Psum.ResizeTo(m_n);
  delete [] m_P;
  m_P = new ZVector[m_n+1];
  for(UCIndex_t i=0; i<=m_n; i++) (m_P[i]).ResizeTo(m_n);
  m_Y.ResizeTo(m_n+1);
  m_PBest.ResizeTo(m_n);
}

Real_t WarmAmoeba::Ooze(UCIndex_t ihi, Real_t& yhi, Real_t fac)
{
  ZVector ptry(m_n);
  Real_t fac1 = (1-fac)/m_n;
  Real_t fac2 = fac1 - fac;
  for(UCIndex_t j=0; j<m_n; j++) { ptry(j) = m_Psum(j)*fac1 - P(ihi,j)*fac2; }
  Real_t ytry = hTrueMaster->Foon(ptry);
  if(ytry <= m_y_best) {
    m_PBest = ptry;
    m_y_best = ytry;
  }
  Real_t yflu = ytry + m_T*TMath::Log(mRanGen.Rndm());
  if(yflu  < yhi) {
    m_Y(ihi) = ytry; yhi = yflu;
    for(UCIndex_t j=0; j<m_n; j++) {
      m_Psum(j) += ptry(j) - P(ihi,j);
      P(ihi,j) = ptry(j);
    }
  }
  return yflu;
}

void WarmAmoeba::WAMove()
{
  Real_t ynhi, ylo, yhi, yt;
  _calc_psum();
  while(1) {
    UCIndex_t ilo=0, ihi=1;
    ynhi = ylo = m_Y(0u) - m_T*TMath::Log(mRanGen.Rndm());
    yhi = m_Y(1u) - m_T*TMath::Log(mRanGen.Rndm());
    if(ylo > yhi) { ihi=0; ilo=1; ynhi=yhi; yhi=ylo; ylo=ynhi; }
    for(UCIndex_t i=2; i<=m_n; i++) {
      yt = m_Y(i) - m_T*TMath::Log(mRanGen.Rndm());
      if(yt <= ylo) { ilo=i; ylo = yt; }
      if(yt > yhi) { ynhi=yhi; ihi=i; yhi=yt; }
      else { ynhi = yt; }
    }
    Real_t rtol = 2*TMath::Abs(yhi-ylo) / (TMath::Abs(yhi) + TMath::Abs(ylo));
    if(rtol < mFTol || m_iter<0) {
      Real_t swap=m_Y(0u); m_Y(0u)=m_Y(ilo); m_Y(ilo)=swap;
      for(UCIndex_t n=0; n<m_n; n++) {
	swap=P(0u,n); P(0u,n)=P(ilo,n); P(ilo,n)=swap;
      }
      break;
    }
    m_iter -= 2;
    Real_t ytry = Ooze(ihi, yhi, -1);
    if(ytry <= ylo) {
      ytry = Ooze(ihi, yhi, 2);
    } else if(ytry >- ynhi) {
      Real_t ysave = yhi;
      ytry = Ooze(ihi, yhi, 0.5);
      if(ytry >= ysave) {
	for(UCIndex_t i=0; i<m_n; i++) {
	  if(i!=ilo) {
	    for(UCIndex_t j=0; j<m_n; j++) {
	      m_Psum(j) = 0.5*(P(i,j) + P(ilo,j));
	      P(i,j) = m_Psum(j);
	    }
	    m_Y(i) = hTrueMaster->Foon(m_Psum);
	  }
	}
	m_iter -= m_n;
	_calc_psum();
      }
    } else { m_iter++; }
  }
}

/**************************************************************************/
// public
/**************************************************************************/

WarmAmoeba::WarmAmoeba(const Text_t* n, const Text_t* t) :
  Eventor(n,t), mWA_Master(0), mRanGen(0)
{ _init(); }

WarmAmoeba::WarmAmoeba(ZGlass* m, const Text_t* n, const Text_t* t) :
  Eventor(n,t), mWA_Master(m), mRanGen(0)
{ _init(); }

WarmAmoeba::~WarmAmoeba()
{
  delete [] m_P;
}

/**************************************************************************/
/**************************************************************************/

Operator::Arg* WarmAmoeba::PreDance(Operator::Arg* op_arg)
{
  op_arg = Eventor::PreDance(op_arg);

  hTrueMaster = dynamic_cast<WarmAmoebaMaster*>(mWA_Master);
  if(!hTrueMaster) {
    ISerr("WarmAmoeba::PreDance master not a WarmAmoebaMaster");
    delete op_arg;
    return 0;
  }
  mRanGen.SetSeed(mSeed);
  { // Init state
    ZVector *v = hTrueMaster->InitialState(mRanGen);
    if(v->GetNoElements() < 3) {
      ISerr("WarmAmoeba::PreDance Need at least 3 parameters for amoeba");
      delete op_arg;
      return 0;
    }
    m_n = v->GetNoElements();
    OP_EXE_OR_SP_MIR(this, SetN, m_n);
    InitZStuff();
    m_P[0] = *v; m_Y(0u) = hTrueMaster->Foon(m_P[0]);
    delete v;
  }
  { // Get perturbations
    ZVector *v = hTrueMaster->InitialPerturbations(mRanGen);
    for(UCIndex_t i=1; i<=m_n; i++) {
      m_P[i] = m_P[0]; P(i, i-1) += (*v)(i-1);
      m_Y(i) = hTrueMaster->Foon(m_P[i]);
    }
    delete v;
  }
  // Estimate temperature
  Real_t t=0;
  for(UCIndex_t i=1; i<=m_n; i++) t += TMath::Abs(m_Y(0u) - m_Y(i));
  m_T = t/m_n;
  m_y_best = mYBest = 10*m_Y(0u); // Large enough
  mYLast = m_Y(0u);
  m_T0 = mTFactor * t/m_n;

  mLocBeatsDone = 0;
  OP_EXE_OR_SP_MIR(this, SetBeatsDone, 0);
  OP_EXE_OR_SP_MIR(this, SetBeatsToDo, mNumSteps);
  OP_EXE_OR_SP_MIR(this, SetT0, m_T0);
  _export_algo_values(op_arg);

  return Eventor::PreDance(op_arg);
}

void WarmAmoeba::PostDance(Operator::Arg* op_arg)
{
  Eventor::PostDance(op_arg);
  _export_algo_values(op_arg, true);
}

/**************************************************************************/

void WarmAmoeba::PostBeat(Operator::Arg* op_arg) throw(Operator::Exception)
{
  Eventor::PostBeat(op_arg);
  if(mStampInterval == 0 || mLocBeatsDone % mStampInterval == 0) {
      _export_algo_values(op_arg);
  }
}

/**************************************************************************/

void WarmAmoeba::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  Eventor::PreOperate(op_arg);

  m_T = m_T0*TMath::Power(1 - (Real_t)mLocBeatsDone/(mBeatsToDo-1), mAlpha);
  m_iter = mMovesPerT;
  WAMove();

  Eventor::PostOperate(op_arg);
}

/**************************************************************************/

#include "WarmAmoeba.c7"
