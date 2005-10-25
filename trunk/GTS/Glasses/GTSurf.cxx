// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTSurf.h"
#include "GTSurf.c7"

#include <GTS/GTS.h>

/**************************************************************************/
/**************************************************************************/

namespace GTS {

  void vertex_dumper(GtsVertex* v) {
    printf("(%4.2f,%4.2f,%4.2f)", v->p.x, v->p.y, v->p.z);
  }

  void segment_dumper(GtsSegment* s) {
    vertex_dumper(s->v1); cout<<"-"; vertex_dumper(s->v2);
  }


  void triangle_dumper(GtsTriangle* t, int* n) {
    cout <<"Tring #"<< *n <<"\t";
    segment_dumper(&t->e1->segment); cout<<" . ";
    segment_dumper(&t->e2->segment); cout<<" . ";
    segment_dumper(&t->e3->segment); cout<<"\n";
    ++(*n);
  }

  void strip_dumper(GSList* tl, int* n) {
    cout <<"Strip #"<< *n <<endl;
    int num = 0;
    g_slist_foreach(tl, (GFunc)triangle_dumper, &num);
    ++(*n);
  }


  int face_inverter(GtsFace* f, int* dum) {
    GtsEdge* egg = f->triangle.e1;
    f->triangle.e1 = f->triangle.e2;
    f->triangle.e2 = egg;
    return 0;
  }
} // gts slurping namespace

/**************************************************************************/
/**************************************************************************/

ClassImp(GTSurf)

void GTSurf::_init()
{
  pSurf = 0; mScale = 1;
  mVerts = mEdges = mFaces = 0;
}

/**************************************************************************/

void GTSurf::ReplaceSurface(GTS::GtsSurface* new_surf)
{
  using namespace GTS;

  if(pSurf) {
    gts_object_destroy (GTS_OBJECT (pSurf));
  }
  pSurf = new_surf;
  mStampReqTring = Stamp(FID());
}

GTS::GtsSurface* GTSurf::CopySurface()
{
  using namespace GTS;

  GtsSurface* s = 0;
  ReadLock();
  if(pSurf) {
    s = MakeDefaultSurface();
    gts_surface_copy(s, pSurf);
  }
  ReadUnlock();
  return s;
}

/**************************************************************************/

void GTSurf::Load()
{
  using namespace GTS;

  FILE* fp = fopen(mFile.Data(), "r");
  if(!fp) {
    ISerr(GForm("GTS::Surface::Load Cant' open %s", mFile.Data()));
    return;
  }
  GtsSurface* s = MakeDefaultSurface();
  if(s==0) {
    ISerr(GForm("GTS::Surface::Load gts_surface_new failed ..."));
    fclose(fp);
    return;
  }
  GtsFile* gsf = gts_file_new(fp);
  if( gts_surface_read(s, gsf) != 0 ) {
    ISerr(GForm("GTS::Surface::Load gts_surface_read failed ..."));
    gts_object_destroy (GTS_OBJECT (s));
    gts_file_destroy(gsf);
    fclose(fp);
    return;
  }
  gts_file_destroy(gsf);
  fclose(fp);

  ReplaceSurface(s);
}

void GTSurf::Save()
{
  using namespace GTS;

  if(pSurf) {
    FILE* fp = fopen(mFile.Data(), "w");
    if(!fp) {
      ISerr(GForm("GTS::Surface::Save Cant' open %s", mFile.Data()));
      return;
    }
    gts_surface_write(pSurf, fp);
    fclose(fp);
  }
}

/**************************************************************************/

namespace {
  using namespace GTS;
  void copy_stats(SGTSRange& d, GtsRange& s) {
    d.SetMin(s.min);  d.SetMax(s.max);
    d.SetAvg(s.mean); d.SetSigma(s.stddev);
  }
}

void GTSurf::CalcStats()
{
  using namespace GTS;

  if(pSurf == 0)
    return;

  mVerts = gts_surface_vertex_number(pSurf);
  mEdges = gts_surface_edge_number(pSurf);
  mFaces = gts_surface_face_number(pSurf);

  GtsSurfaceQualityStats stats;
  // pSurf->CalcStats(mFaceQuality, mFaceArea, mEdgeLength, mEdgeAngle);
  gts_surface_quality_stats(pSurf, &stats);
  copy_stats(mFaceQuality, stats.face_quality);
  copy_stats(mFaceArea,    stats.face_area);
  copy_stats(mEdgeLength,  stats.edge_length);
  copy_stats(mEdgeAngle,   stats.edge_angle);

  Stamp(FID());
}

void GTSurf::PrintStats()
{
  if(pSurf) {
    GTS::gts_surface_print_stats(pSurf, stdout);
  }
}

/**************************************************************************/

void GTSurf::Destroy()
{
  using namespace GTS;

  if(pSurf == 0)
    return;

  gts_object_destroy (GTS_OBJECT (pSurf));
  pSurf = 0;
  mStampReqTring = Stamp(FID());
}

void GTSurf::Invert()
{
  using namespace GTS;

  if(pSurf) {
    gts_surface_foreach_face(pSurf, (GtsFunc)face_inverter, 0);
    mStampReqTring = Stamp(FID());
  }
}

/**************************************************************************/

void GTSurf::Tessellate(UInt_t order)
{
  if(pSurf == 0)
    return;

  while(order--)
    gts_surface_tessellate(pSurf, 0, 0);

  mStampReqTring = Stamp(FID());
}

void GTSurf::GenerateSphere(UInt_t order)
{
  if(pSurf)
    gts_object_destroy (GTS_OBJECT (pSurf));
  pSurf = MakeDefaultSurface();

  gts_surface_generate_sphere(pSurf, order);

  mStampReqTring = Stamp(FID());
}

/**************************************************************************/
/**************************************************************************/

#include <TRandom3.h>
#include <TVector3.h>

namespace {

class Legend
{
protected:
  Double_t         mCosMPhi, mSinMPhi; // Buffers during evaluation

public:
  Int_t            mL;
  vector<Double_t> mC;

  Legend(Int_t max_l,  Double_t abs_scale, Double_t pow_scale) :
    mL(max_l), mC((max_l+1)*(max_l+1))
  {
    using namespace TMath;

    TRandom3 rnd(0);

    for(int l=0; l<=mL; ++l) {
      Double_t fl   = 0.25*(2*l + 1)/Pi();
      Double_t fpow = abs_scale * Power(1.0/(l + 1), pow_scale);

      Double_t& l0 = Coff(l, 0);

      l0  = Sqrt(fl);
      l0 *= fpow * (2*rnd.Rndm() - 1);
      // printf("l=%d 0=-%.6f", l, l0);

      for(int m=1; m<=l; ++m) {
	fl /= (l+m)*(l-m+1);

	Double_t& lpos = Coff(l,  m);
	Double_t& lneg = Coff(l, -m);

	lpos  = Sqrt(fl);
	lneg  = (m % 2) ? -lpos : lpos;
	lpos *= fpow * (2*rnd.Rndm() - 1);
	lneg *= fpow * (2*rnd.Rndm() - 1);
	// printf(" %d=%-.6f %d=%-.6f", m, lpos, -m, lneg);
      }
      // printf("\n");
    }
  }

  inline Double_t& Coff(int l, int m) { return mC[l*l + l + m]; }

  inline Double_t  SumM(int l, int m)
  {
    if(m == 0) {
      return mC[l*l + l];
    } else {
      int idx = l*l + l - m;
      return mC[idx + 2*m] * mCosMPhi + mC[idx] * mSinMPhi;
    }
  }

  //----------------------------------------------------------------

  void vertex_displacer(GTS::GtsVertex* v)
  {
    TVector3 vec(v->p.x, v->p.y, v->p.z);

    Double_t phi = vec.Phi();
    Double_t x   = vec.CosTheta();

    Double_t somx2 = TMath::Sqrt((1.0-x)*(1.0+x));
    Double_t fact  = 1;
    Double_t Pmm   = 1;

    Double_t sum = 1;

    for(int m=0; m<=mL; ++m) {
      mCosMPhi = TMath::Cos(m*phi);
      mSinMPhi = TMath::Sin(m*phi);

      sum += Pmm * SumM(m, m);

      if(m < mL) {
	Double_t Pam = Pmm;
	Double_t Pbm = x*(2*m+1)*Pam;

	sum += Pbm * SumM(m+1, m);

	for(int l=m+2; l<=mL; ++l) {
	  Double_t Plm = (x*(2*l-1)*Pbm - (l+m-1)*Pam) / (l-m);

	  sum += Plm * SumM(l, m);

	  Pam = Pbm;
	  Pbm = Plm;
	}
      }

      // Calc Pm,m+1
      Pmm  *= -fact*somx2;
      fact +=  2.0;
    }

    vec *= sum;

    v->p.x = vec.x(); v->p.y = vec.y(); v->p.z = vec.z();
  }
  
  static void s_vertex_displacer(GTS::GtsVertex* v, Legend* ud)
  {
    ud->vertex_displacer(v);
  }

}; // endclass Legend

} // end namespace

/**************************************************************************/

void GTSurf::Legendrofy(Int_t max_l, Double_t abs_scale, Double_t pow_scale)
{
  if(pSurf == 0)
    return;

  Legend leg(max_l, abs_scale, pow_scale);

  GTS::gts_surface_foreach_vertex(pSurf,
				  (GTS::GtsFunc)Legend::s_vertex_displacer,
				  &leg);

  mStampReqTring = Stamp(FID());
}


// Original LegendreP
/*
Double_t LegendreP(int l, int m, Double_t x)
{
  void nrerror(char error_text[]);
  Double_t fact,pll,pmm,pmmp1,somx2;
  int i,ll;

  if (m < 0 || m > l || TMath::Abs(x) > 1.0)
    throw(Exc_t("LegendreP: bad arguments.");
  pmm=1.0;
  if (m > 0) {
    somx2=sqrt((1.0-x)*(1.0+x));
    fact=1.0;
    for (i=1;i<=m;i++) {
      pmm *= -fact*somx2;
      fact += 2.0;
    }
  }
  if (l == m)
    return pmm;
  else {
    pmmp1=x*(2*m+1)*pmm;
    if (l == (m+1))
      return pmmp1;
    else {
      for (ll=m+2;ll<=l;ll++) {
	pll=(x*(2*ll-1)*pmmp1-(ll+m-1)*pmm)/(ll-m);
	pmm=pmmp1;
	pmmp1=pll;
      }
      return pll;
    }
  }
}
*/
