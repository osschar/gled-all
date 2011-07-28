// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTSurf.h"
#include <Glasses/LegendreCoefs.h>
#include "GTSurf.c7"

#include <GTS/GTS.h>
#include <GTS/GTSBoolOpHelper.h>

#include <TMath.h>
#include <TRandom3.h>
#include <TTree.h>

namespace { using namespace GTS; }

//______________________________________________________________________________
//
// Wrapper over GTS surface.


ClassImp(GTSurf);

void GTSurf::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  mColor.rgba(1, 1, 1, 1);
  mPointColor.rgba(1, 0, 0, 1);
  bRnrPoints = false;

  pSurf = 0;
  mVerts = mEdges = mFaces = 0;

  mPostBoolOp = PBM_AsFractions;
  mPostBoolArea      = 1e-8;
  mPostBoolPerimeter = 1e-8;
  mPostBoolLength    = 1e-1;
}

/**************************************************************************/

void GTSurf::ReplaceSurface(GTS::GtsSurface* new_surf)
{
  using namespace GTS;

  GLensReadHolder _lck(this);
  if (pSurf)
  {
    gts_object_destroy(GTS_OBJECT(pSurf));
  }
  pSurf = new_surf;
  mStampReqTring = Stamp(FID());
}

GTS::GtsSurface* GTSurf::CopySurface()
{
  GLensReadHolder _lck(this);

  if (pSurf == 0) return 0;

  GTS::GtsSurface* s = GTS::MakeDefaultSurface();
  GTS::gts_surface_copy(s, pSurf);

  return s;
}

GTS::GtsSurface* GTSurf::DisownSurface()
{
  GLensReadHolder _lck(this);

  GTS::GtsSurface* s = pSurf;
  pSurf = 0;
  mStampReqTring = Stamp(FID());

  return s;
}


//==============================================================================

namespace
{
  struct projected_area_sum_arg
  {
    HPointD dir;
    double  sum;

    projected_area_sum_arg(double x, double y, double z) : dir(x,y,z), sum(0) {}
  };

  void projected_area_sum(GtsFace* f, projected_area_sum_arg* arg)
  {
    // This is summing *twice* the area of each triangle.

    HPointD p;
    gts_triangle_normal(&f->triangle, &p.x, &p.y, &p.z);
    arg->sum += TMath::Abs(arg->dir.Dot(p));
  }
}

Double_t GTSurf::GetArea() const
{
  if (!pSurf) return 0;
  return gts_surface_area(pSurf);
}

Double_t GTSurf::GetXYArea() const
{
  if (!pSurf) return 0;

  projected_area_sum_arg arg(0, 0, 1);
  gts_surface_foreach_face(pSurf, (GtsFunc) projected_area_sum, &arg);
  return arg.sum / 4;
}

Double_t GTSurf::GetVolume() const
{
  if (!pSurf) return 0;
  return gts_surface_volume(pSurf);
}


//==============================================================================

void GTSurf::Load(const TString& file)
{
  static const Exc_t _eh("GTSurf::Load ");

  using namespace GTS;

  TString file_name = file.IsNull() ? mFile : file;

  FILE* fp = fopen(file_name, "r");
  if (!fp)
  {
    ISerr(_eh + GForm("Can not open file '%s'", file_name.Data()));
    return;
  }

  GtsSurface *s   = MakeDefaultSurface();
  GtsFile    *gsf = gts_file_new(fp);
  if (gts_surface_read(s, gsf) != 0)
  {
    ISerr(_eh + GForm("gts_surface_read failed."));
    gts_object_destroy(GTS_OBJECT(s));
    gts_file_destroy(gsf);
    fclose(fp);
    return;
  }
  gts_file_destroy(gsf);
  fclose(fp);

  ReplaceSurface(s);
}

void GTSurf::Save(const TString& file)
{
  static const Exc_t _eh("GTSurf::Save ");

  using namespace GTS;

  if (pSurf == 0) {
    ISerr(_eh + "Surface is null.");
    return;
  }

  TString file_name = file.IsNull() ? mFile : file;

  FILE* fp = fopen(file_name, "w");
  if (!fp) {
    ISerr(_eh + GForm("Can not open file '%s'.", file_name.Data()));
    return;
  }
  gts_surface_write(pSurf, fp);
  fclose(fp);
}


//==============================================================================

namespace
{
  void copy_stats(SGTSRange& d, GTS::GtsRange& s)
  {
    d.SetMin(s.min);  d.SetMax(s.max);
    d.SetAvg(s.mean); d.SetSigma(s.stddev);
  }
}

void GTSurf::CalcStats()
{
  using namespace GTS;

  if (pSurf == 0) return;

  mVerts = gts_surface_vertex_number(pSurf);
  mEdges = gts_surface_edge_number(pSurf);
  mFaces = gts_surface_face_number(pSurf);

  GtsSurfaceQualityStats stats;
  gts_surface_quality_stats(pSurf, &stats);
  copy_stats(mFaceQuality, stats.face_quality);
  copy_stats(mFaceArea,    stats.face_area);
  copy_stats(mEdgeLength,  stats.edge_length);
  copy_stats(mEdgeAngle,   stats.edge_angle);

  Stamp(FID());
}

void GTSurf::PrintStats()
{
  if (pSurf) {
    GTS::gts_surface_print_stats(pSurf, stdout);
  }
}

/**************************************************************************/

void GTSurf::Destroy()
{
  using namespace GTS;

  if (pSurf == 0) return;

  gts_object_destroy (GTS_OBJECT (pSurf));
  pSurf = 0;
  mStampReqTring = Stamp(FID());
}

void GTSurf::Invert()
{
  using namespace GTS;

  if (pSurf) 
  {
    InvertSurface(pSurf);
    mStampReqTring = Stamp(FID());
  }
}

/**************************************************************************/

namespace
{
  void vertex_scaler(GtsVertex* v, Double_t* s)
  {
    v->p.x *= s[0]; v->p.y *= s[1]; v->p.z *= s[2];
  }
}

void GTSurf::Rescale(Double_t s)
{
  using namespace GTS;

  if (pSurf)
  {
    Double_t sxyz[3] = { s, s, s };
    gts_surface_foreach_vertex(pSurf, (GtsFunc)vertex_scaler, &sxyz);
    mStampReqTring = Stamp(FID());
  }
}

void GTSurf::RescaleXYZ(Double_t sx, Double_t sy, Double_t sz)
{
  using namespace GTS;

  if (pSurf)
  {
    Double_t sxyz[3] = { sx, sy, sz };
    gts_surface_foreach_vertex(pSurf, (GtsFunc)vertex_scaler, &sxyz);
    mStampReqTring = Stamp(FID());
  }
}

void GTSurf::TransformAndResetTrans()
{
  if (pSurf)
    GTS::TransformSurfaceVertices(pSurf, &mTrans);
  UnitTrans();
}

void GTSurf::RotateAndResetRot()
{
  if (pSurf)
    GTS::RotateSurfaceVertices(pSurf, &mTrans);
  UnitRot();
}


//==============================================================================

namespace
{
  GtsVertex* mid_edge_splitter(GtsEdge *e, GtsVertexClass *k, gpointer /*d*/)
  {
    GtsPoint &a = e->segment.v1->p, &b = e->segment.v2->p;
    return gts_vertex_new(k, a.x + 0.5*(b.x - a.x),
			  a.y + 0.5*(b.y - a.y),
			  a.z + 0.5*(b.z - a.z));
  }
}

void GTSurf::Tessellate(UInt_t order, Bool_t mid_edge)
{
  if (pSurf == 0) return;

  while (order--)
  {
    gts_surface_tessellate(pSurf, mid_edge ? mid_edge_splitter : 0, 0);
  }
  mStampReqTring = Stamp(FID());
}

//==============================================================================

void GTSurf::Merge(GTSurf* a, GTSurf* b)
{
  // Merge surfaces a and b into current surface.
  // a or b can be null.
  // Vertices of a and b are transformed into local coordinate system.

  static const Exc_t _eh("GTSurf::Merge ");

  GTS::BoolOpHelper boh(this, a, b, _eh);
  boh.MakeMerge();
  ReplaceSurface(boh.TakeResult());
}

void GTSurf::Union(GTSurf* a, GTSurf* b)
{
  // Merge union of a and b into this surface.
  // Vertices of a and b are transformed into local coordinate system.

  static const Exc_t _eh("GTSurf::Union ");

  GTS::BoolOpHelper boh(this, a, b, _eh);
  boh.BuildInter(_eh);
  boh.MakeUnion();
  ReplaceSurface(boh.TakeResult());
}

void GTSurf::Intersection(GTSurf* a, GTSurf* b)
{
  // Merge intersection of a and b into this surface.
  // Vertices of a and b are transformed into local coordinate system.

  static const Exc_t _eh("GTSurf::Intersection ");

  GTS::BoolOpHelper boh(this, a, b, _eh);
  boh.BuildInter(_eh);
  boh.MakeIntersection();
  ReplaceSurface(boh.TakeResult());
}

void GTSurf::Difference(GTSurf* a, GTSurf* b)
{
  // Merge difference of a and b into this surface.
  // Vertices of a and b are transformed into local coordinate system.

  static const Exc_t _eh("GTSurf::Difference ");

  GTS::BoolOpHelper boh(this, a, b, _eh);
  boh.BuildInter(_eh);
  boh.MakeDifference();
  ReplaceSurface(boh.TakeResult());
}

//==============================================================================

void GTSurf::GenerateSphere(UInt_t order)
{
  if (pSurf)
    gts_object_destroy (GTS_OBJECT (pSurf));
  pSurf = MakeDefaultSurface();

  gts_surface_generate_sphere(pSurf, order);

  mStampReqTring = Stamp(FID());
}

void GTSurf::GenerateTriangle(Double_t s)
{
  if (pSurf)
    gts_object_destroy (GTS_OBJECT (pSurf));
  pSurf = MakeDefaultSurface();

  const Double_t sqrt3 = TMath::Sqrt(3);
  GtsVertex * v[3];
  v[0] = gts_vertex_new(pSurf->vertex_class, -s*0.5, -s*sqrt3/6, 0);
  v[1] = gts_vertex_new(pSurf->vertex_class,  s*0.5, -s*sqrt3/6, 0);
  v[2] = gts_vertex_new(pSurf->vertex_class,  0,      s*sqrt3/3, 0);

  GtsEdge * e[3];
  e[0] = gts_edge_new(pSurf->edge_class, v[0], v[1]);
  e[1] = gts_edge_new(pSurf->edge_class, v[1], v[2]);
  e[2] = gts_edge_new(pSurf->edge_class, v[2], v[0]);

  GtsFace * f = gts_face_new(pSurf->face_class, e[0], e[1], e[2]);
  gts_surface_add_face(pSurf, f);

  mStampReqTring = Stamp(FID());
}


//==============================================================================
// Legendrification
//==============================================================================

namespace
{
  void legendre_vertex_adder(GTS::GtsVertex* v, LegendreCoefs::Evaluator* e)
  {
    HPointD  vec(v->p.x, v->p.y, v->p.z);
    vec *= 1.0 + e->Eval(vec) / vec.Mag();
    v->p.x = vec.x; v->p.y = vec.y; v->p.z = vec.z;
  }

  void legendre_vertex_scaler(GTS::GtsVertex* v, LegendreCoefs::Evaluator* e)
  {
    HPointD  vec(v->p.x, v->p.y, v->p.z);
    vec *= 1.0 + e->Eval(vec);
    v->p.x = vec.x; v->p.y = vec.y; v->p.z = vec.z;
  }
}

void GTSurf::LegendrofyAdd(LegendreCoefs* lc, Double_t scale, Int_t l_max)
{
  static const Exc_t _eh("GTSurf::LegendrofyAdd ");

  if (pSurf == 0) throw _eh + "member pSurf is 0.";
  if (lc    == 0) throw _eh + "argument lc is 0.";

  LegendreCoefs::Evaluator eval(lc, scale, l_max);

  GTS::gts_surface_foreach_vertex(pSurf,
				  (GTS::GtsFunc) legendre_vertex_adder,
				  &eval);

  mStampReqTring = Stamp(FID());
}

void GTSurf::LegendrofyScale(LegendreCoefs* lc, Double_t scale, Int_t l_max)
{
  static const Exc_t _eh("GTSurf::LegendrofyScale ");

  if (pSurf == 0) throw _eh + "member pSurf is 0.";
  if (lc    == 0) throw _eh + "argument lc is 0.";

  LegendreCoefs::Evaluator eval(lc, scale, l_max);

  GTS::gts_surface_foreach_vertex(pSurf,
				  (GTS::GtsFunc) legendre_vertex_scaler,
				  &eval);

  mStampReqTring = Stamp(FID());
}

void GTSurf::LegendrofyScaleRandom(Int_t l_max, Double_t abs_scale, Double_t pow_scale)
{
  // Single-shopping wrapper -- creates a dummy LegendreCoefs lens without
  // enlightening it.

  static const Exc_t _eh("GTSurf::LegendrofyScaleRandom ");

  if (pSurf == 0) throw _eh + "member pSurf is 0.";

  auto_ptr<LegendreCoefs> lc(new LegendreCoefs);
  lc->InitRandom(l_max, abs_scale, pow_scale);
  lc->SetCoef(0, 0, 0);

  LegendreCoefs::Evaluator eval(lc.get());

  GTS::gts_surface_foreach_vertex(pSurf,
				  (GTS::GtsFunc) legendre_vertex_scaler,
				  &eval);

  mStampReqTring = Stamp(FID());
}

//------------------------------------------------------------------------------
// Legendrification, the Multi way
//------------------------------------------------------------------------------

namespace
{
  void lcme_filler(GTS::GtsVertex* v, LegendreCoefs::MultiEval* me)
  {
    me->AddPoint(v->p.x, v->p.y, v->p.z, v);
  }
}

void GTSurf::legendrofy_multi_common(LegendreCoefs* lc, LegendreCoefs::MultiEval& me, const Exc_t eh)
{
  if (pSurf == 0) throw eh + "member pSurf is 0.";
  if (lc    == 0) throw eh + "argument lc is 0.";

  me.Init(gts_surface_vertex_number(pSurf));
  gts_surface_foreach_vertex(pSurf, (GtsFunc) lcme_filler, &me);
  me.Sort();
}

void GTSurf::LegendrofyAddMulti(LegendreCoefs* lc, Double_t scale, Int_t l_max)
{
  static const Exc_t _eh("GTSurf::LegendrofyAddMulti ");

  LegendreCoefs::MultiEval me;

  legendrofy_multi_common(lc, me, _eh);

  lc->EvalMulti(me, l_max);

  for (Int_t i = 0; i < me.fN; ++i)
  {
    GTS::GtsVertex *v = (GTS::GtsVertex*) me.fUserData[i];
    HPointD  vec(v->p.x, v->p.y, v->p.z);
    vec *= 1.0 + scale * me.fMVec[i] / vec.Mag();
    v->p.x = vec.x; v->p.y = vec.y; v->p.z = vec.z;
  }

  mStampReqTring = Stamp(FID());
}

void GTSurf::LegendrofyScaleMulti(LegendreCoefs* lc, Double_t scale, Int_t l_max)
{
  static const Exc_t _eh("GTSurf::LegendrofyScaleMulti ");

  LegendreCoefs::MultiEval me;

  legendrofy_multi_common(lc, me, _eh);

  lc->EvalMulti(me, l_max);

  for (Int_t i = 0; i < me.fN; ++i)
  {
    GTS::GtsVertex *v = (GTS::GtsVertex*) me.fUserData[i];
    HPointD  vec(v->p.x, v->p.y, v->p.z);
    vec *= 1.0 + scale * me.fMVec[i];
    v->p.x = vec.x; v->p.y = vec.y; v->p.z = vec.z;
  }

  mStampReqTring = Stamp(FID());
}

void GTSurf::LegendrofyRandomMulti(Int_t l_max, Double_t abs_scale, Double_t pow_scale)
{
  // Expects points at R = 1.

  static const Exc_t _eh("GTSurf::LegendrofyRandomMulti ");

  auto_ptr<LegendreCoefs> lc(new LegendreCoefs);
  lc->InitRandom(l_max, abs_scale, pow_scale);
  lc->SetCoef(0, 0, 0);

  LegendreCoefs::MultiEval me;

  legendrofy_multi_common(lc.get(), me, _eh);

  lc->EvalMulti(me, l_max);

  for (Int_t i = 0; i < me.fN; ++i)
  {
    GTS::GtsVertex *v = (GTS::GtsVertex*) me.fUserData[i];
    const Double_t fac = 1.0 + me.fMVec[i];
    v->p.x *= fac; v->p.y *= fac; v->p.z *= fac;
  }

  mStampReqTring = Stamp(FID());
}


//==============================================================================
// Triangle exporter
//==============================================================================

namespace
{
  struct extring_arg
  {
    map<GtsVertex*, int> m_map;
    int                  m_count;
    FILE*                m_out;

    extring_arg() : m_count(0), m_out(0) {}
  };

  void trivi_vdump(GtsVertex* v, extring_arg* arg)
  {
    arg->m_map[v] = arg->m_count;
    ++arg->m_count;
    fprintf(arg->m_out, "%lf %lf %lf\n", v->p.x, v->p.y, v->p.z);
  }

  void trivi_fdump(GtsFace* f, extring_arg* arg)
  {
    GtsVertex *a, *b, *c;
    gts_triangle_vertices(&f->triangle, &a, &b, &c);
    fprintf(arg->m_out, "%d %d %d\n", arg->m_map[a], arg->m_map[b], arg->m_map[c]);
  }
}

void GTSurf::ExportTring(const Text_t* fname)
{
  // Dumps vertices/triangles in a trivial format.

  using namespace GTS;

  if (pSurf == 0) return;

  FILE* f = (fname) ? fopen(fname, "w") : stdout;

  fprintf(f, "%u %u\n", gts_surface_vertex_number(pSurf),
                        gts_surface_face_number(pSurf));

  extring_arg arg;
  arg.m_out = f;

  gts_surface_foreach_vertex(pSurf, (GtsFunc) trivi_vdump, &arg);
  gts_surface_foreach_face  (pSurf, (GtsFunc) trivi_fdump, &arg);

  if (fname) fclose(f);
}


//==============================================================================
// Making of split surfaces
//==============================================================================

namespace
{
  void vertex_min_z_fixer(GTS::GtsVertex* v, Double_t* min_z)
  {
    if (v->p.z < *min_z) v->p.z = *min_z;
  }

  void vertex_max_z_fixer(GTS::GtsVertex* v, Double_t* max_z)
  {
    if (v->p.z > *max_z) v->p.z = *max_z;
  }

}

void GTSurf::MakeZSplitSurfaces(Double_t z_split, const TString& stem, Bool_t save_p)
{
  static const Exc_t _eh("GTSurf::SaveZSplitSurfaces ");

  using namespace GTS;

  GtsSurface *sup = 0, *sdn = 0;

  {
    GLensReadHolder _rdlck(this);

    if (!pSurf)
      throw _eh + "Surface is null.";

    sup = CopySurface();
    sdn = CopySurface();
  }

  gts_surface_foreach_vertex(sup, (GtsFunc) vertex_min_z_fixer, &z_split);
  gts_surface_foreach_vertex(sdn, (GtsFunc) vertex_max_z_fixer, &z_split);

  GTSurf *gsup = new GTSurf(GForm("Upper %s", GetName()));
  gsup->ReplaceSurface(sup);
  gsup->SetFile(GForm("%s-upper.gts", stem.Data()));

  GTSurf *gsdn = new GTSurf(GForm("Lower %s", GetName()));
  gsdn->ReplaceSurface(sdn);
  gsdn->SetFile(GForm("%s-lower.gts", stem.Data()));

  {
    GLensWriteHolder _wrlck(this);

    mQueen->CheckIn(gsup);
    mQueen->CheckIn(gsdn);

    Add(gsup);
    Add(gsdn);
  }

  if (save_p)
  {
    { GLensReadHolder _rdlck(gsup); gsup->Save(); }
    { GLensReadHolder _rdlck(gsdn); gsdn->Save(); }
  }
}

//==============================================================================

namespace
{
  struct ct_ud
  {
    TTree    *t;
    HPointD  *p;
    ct_ud() : t(0), p(0) {}
  };

  void ct_filler(GTS::GtsVertex* v, ct_ud* ud)
  {
    ud->p->Set(v->p.x, v->p.y, v->p.z);
    ud->t->Fill();
  }
}

TTree* GTSurf::MakeHPointDTree(const TString& name, const TString& title)
{
  if (pSurf == 0) return 0;

  ct_ud ud;

  TTree *t = new TTree(name, title);
  t->SetDirectory(0);

  ud.t = t;
  t->Branch("P", &ud.p);

  gts_surface_foreach_vertex(pSurf, (GtsFunc) ct_filler, &ud);

  return ud.t;
}

TTree* GTSurf::MakeMultiEvalTree(const TString& name, const TString& title)
{
  if (pSurf == 0) return 0;

  LegendreCoefs::MultiEval me;
  me.Init(gts_surface_vertex_number(pSurf));
  gts_surface_foreach_vertex(pSurf, (GtsFunc) lcme_filler, &me);
  me.Sort();

  TTree *t = new TTree(name, title);
  t->SetDirectory(0);

  Double_t ct, d;
  t->Branch("B1", &ct, "ct/D");
  t->Branch("B2", &d,  "d/D");

  for (Int_t i = 1; i < me.fN; ++i)
  {
    Int_t i1 = me.fIdcs[i];
    Int_t i0 = me.fIdcs[i - 1];

    ct = me.fMVec[i1];
    d  = me.fMVec[i1] - me.fMVec[i0];
    t->Fill();
  }

  return t;
}
