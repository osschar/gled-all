// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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

  if(pSurf) {
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
  }
  Stamp(FID());
}

void GTSurf::PrintStats()
{
  if(pSurf) {
    GTS::gts_surface_print_stats(pSurf, stdout);
  }
}

/**************************************************************************/

void GTSurf::Invert()
{
  using namespace GTS;

  if(pSurf) {
    gts_surface_foreach_face(pSurf, (GtsFunc)face_inverter, 0);
    mStampReqTring = Stamp(FID());
  }
}

/**************************************************************************/
