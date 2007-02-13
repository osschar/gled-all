// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TriMesh
//
// Wrapper over TringTvor (triangulation data) and Opcode structures.
//
// User is responsible for (re)creation of aabboxes in TringTvor.
//
// Structure VConnData must be reduced (possibly use quantization to shorts.
// Structure VertexData should be extended with per-point values.
// I guess something like gl uniform varibles (including vector forms).


#include "TriMesh.h"
#include <Glasses/RectTerrain.h>
#include <Glasses/GTSurf.h>
#include <Glasses/ZImage.h>
#include "TriMesh.c7"

#include <GTS/GTS.h>

#include <Opcode/Opcode.h>

ClassImp(TriMesh)

/**************************************************************************/

void TriMesh::_init()
{
  mTTvor     = 0;
  mOPCModel  = 0;
  mOPCMeshIf = 0;
}

TriMesh::~TriMesh()
{
  delete mTTvor;
  delete mOPCModel;
  delete mOPCMeshIf;
}

void TriMesh::ResetTTvorDependants()
{
  delete mOPCModel;  mOPCModel  = 0;
  delete mOPCMeshIf; mOPCMeshIf = 0;

  mVDataVec.resize(0);
}

/**************************************************************************/

void TriMesh::StdSurfacePostImport()
{
  CalculateBoundingBox();
  BuildOpcStructs();

  BuildVertexConnections();
}

void TriMesh::StdDynamicoPostImport()
{
  CalculateBoundingBox();
  BuildOpcStructs();
}

/**************************************************************************/

void TriMesh::BuildOpcStructs()
{
  static const Exc_t _eh("TriMesh::BuildOpcStructs ");

  if(!mTTvor)
    throw(_eh + "mTTvor is null.");

  using namespace Opcode;

  delete mOPCModel;
  delete mOPCMeshIf;
  mOPCModel  = new Model;
  mOPCMeshIf = new MeshInterface;
  mOPCMeshIf->SetNbTriangles(mTTvor->mNTrings);
  mOPCMeshIf->SetNbVertices(mTTvor->mNVerts);
  mOPCMeshIf->SetPointers((IndexedTriangle*) mTTvor->mTrings,
                                    (Point*) mTTvor->mVerts);

  OPCODECREATE OPCC;
  OPCC.mIMesh = mOPCMeshIf;

  bool status = mOPCModel->Build(OPCC);
  printf("Build finished rstatus=%d, bytes=%d\n", status, mOPCModel->GetUsedBytes());
}

void TriMesh::AssertOpcStructs()
{
  if (mOPCModel == 0 || mOPCMeshIf == 0)
    BuildOpcStructs();
}

/**************************************************************************/
// TringTvor interface
/**************************************************************************/

void TriMesh::CalculateBoundingBox()
{
  static const Exc_t _eh("TriMesh::CalculateBoundingBox ");

  if (!mTTvor)
    throw(_eh + "mTTvor is null.");

  mTTvor->CalculateBoundingBox();
  Float_t *b = mTTvor->mMinMaxBox, *e = mTTvor->mCtrExtBox;
  printf("%s %s\n"
         "  min = % 8.4f, % 8.4f, % 8.4f  max = % 8.4f, % 8.4f, % 8.4f\n"
         "  ctr = % 8.4f, % 8.4f, % 8.4f  ext = % 8.4f, % 8.4f, % 8.4f\n",
         _eh.Data(), Identify().Data(),
         b[0], b[1], b[2], b[3], b[4], b[5],
         e[0], e[1], e[2], e[3], e[4], e[5]);

  // mStampReqTring = Stamp(FID());
}

void TriMesh::GenerateVertexNormals()
{
  static const Exc_t _eh("TriMesh::GenerateVertexNormals ");

  if (!mTTvor)
    throw(_eh + "mTTvor is null.");

  mTTvor->GenerateVertexNormals();
  mStampReqTring = Stamp(FID());
}

/**************************************************************************/
// RectTerrain import
/**************************************************************************/

void TriMesh::ImportRectTerrain(RectTerrain* rt, Bool_t colp, Bool_t texp)
{
  static const Exc_t _eh("TriMesh::ImportRectTerrain ");

  if(rt == 0)
    throw(_eh + "null argument passed.");

  TringTvor* tt = 0;
  { GLensReadHolder _rlck(rt);
    printf("lock terrain\n");
    tt = rt->SpawnTringTvor(true, true, colp, texp);
    if(tt == 0)
      throw(_eh + "tvor null after MakeTringTvor().");
    printf("unlock terrain\n");
  }
  GLensWriteHolder _wlck(this);
  printf("lock tringula\n");
  delete mTTvor;
  mTTvor = tt;
  mStampReqTring = Stamp(FID());
  printf("unlock tringula\n");
}

/**************************************************************************/
// GTSurf import/export
/**************************************************************************/

void TriMesh::ImportGTSurf(GTSurf* gts)
{
  static const Exc_t _eh("TriMesh::ImportGTSurf ");

  using namespace GTS;

  if (!gts)
    throw(_eh + "called with null argument.");
  if(!gts->GetSurf())
    throw(_eh + "GtsSurface is null.");

  struct Dumper
  {
    map<GtsVertex*, int>  map;
    TringTvor            *tvor;
    int                   vcount;
    int                   tcount;

    Dumper(TringTvor* t) : tvor(t), vcount(0), tcount(0) {}

    static void vdump(GtsVertex* v, Dumper* arg)
    {
      arg->map[v] = arg->vcount;
      Float_t *q = arg->tvor->Vertex(arg->vcount);
      q[0] = v->p.x; q[1] = v->p.y; q[2] = v->p.z;
      ++arg->vcount;
    }

    static void fdump(GtsFace* f, Dumper* arg)
    {
      GtsVertex *a, *b, *c;
      gts_triangle_vertices(&f->triangle, &a, &b, &c);
      Int_t *q = arg->tvor->Triangle(arg->tcount);
      q[0] = arg->map[a]; q[1] = arg->map[b]; q[2] = arg->map[c];
      ++arg->tcount;
    }

  };

  GtsSurface* surf = gts->GetSurf();
  TringTvor* tt = new TringTvor(gts_surface_vertex_number(surf),
                                gts_surface_face_number  (surf));

  Dumper arg(tt);
  gts_surface_foreach_vertex(surf, (GtsFunc) Dumper::vdump, &arg);
  gts_surface_foreach_face  (surf, (GtsFunc) Dumper::fdump, &arg);

  // tt->MakeSecondaryArrays(true,  true, false);
  // tt->MakeSecondaryArrays(false, true, false);
  tt->GenerateVertexNormals();
  tt->GenerateTriangleNormals();

  GLensWriteHolder _wlck(this);
  printf("lock tringula\n");
  delete mTTvor;
  mTTvor = tt;
  mStampReqTring = Stamp(FID());
  printf("unlock tringula\n");
}


namespace {
struct xx_edge
{
  Int_t v1, v2;
  xx_edge(Int_t _v1, Int_t _v2) : v1(_v1), v2(_v2) {}
  bool operator==(const xx_edge& o) const
  { return (o.v1==v1 && o.v2==v2) || (o.v1==v2 && o.v2==v1); }
};
}
namespace __gnu_cxx {
template<> struct hash<xx_edge> {
  size_t operator()(const xx_edge& xx) const
  { size_t i = xx.v1 * xx.v2; return i; }
};
}

void TriMesh::ExportGTSurf(GTSurf* gts)
{
  using namespace GTS;

  // Prepare edge data

  typedef hash_map<xx_edge, Int_t>           hmap_t;
  typedef hash_map<xx_edge, Int_t>::iterator hmap_i;

  hmap_t edge_map;
  Int_t  edge_cnt = 0;
  {
    Int_t* ta = mTTvor->mTrings;
    for(Int_t t=0; t<mTTvor->mNTrings; ++t) {
      pair<hmap_i, bool> res;
      res = edge_map.insert(make_pair(xx_edge(ta[0], ta[1]), edge_cnt));
      if(res.second) ++edge_cnt;
      res = edge_map.insert(make_pair(xx_edge(ta[1], ta[2]), edge_cnt));
      if(res.second) ++edge_cnt;
      res = edge_map.insert(make_pair(xx_edge(ta[2], ta[0]), edge_cnt));
      if(res.second) ++edge_cnt;
      ta += 3;
    }
  }
  printf("Counting edges => %d, map-size = %d\n", edge_cnt, (int)edge_map.size());

  // Construct surface

  GtsSurface* surf = MakeDefaultSurface();

  GtsVertex **vertices = new GtsVertex*[mTTvor->mNVerts];
  {
    Float_t* va = mTTvor->mVerts;
    for(Int_t v=0; v<mTTvor->mNVerts; ++v) {
      vertices[v] = gts_vertex_new(surf->vertex_class,
                                   va[0], va[1], va[2]);
      va += 3;
    }
  }

  GtsEdge ** edges = new GtsEdge*[edge_cnt];
  {
    for(hmap_i e=edge_map.begin(); e!=edge_map.end(); ++e) {
      edges[e->second] = gts_edge_new(surf->edge_class,
                                      vertices[e->first.v1],
                                      vertices[e->first.v2]);
    }
  }

  {
    Int_t* ta = mTTvor->mTrings;
    for(Int_t t=0; t<mTTvor->mNTrings; ++t) {
      Int_t e1, e2, e3;
      e1 = edge_map.find(xx_edge(ta[0], ta[1]))->second;
      e2 = edge_map.find(xx_edge(ta[1], ta[2]))->second;
      e3 = edge_map.find(xx_edge(ta[2], ta[0]))->second;
      GtsFace * new_face = gts_face_new(surf->face_class,
                                        edges[e1], edges[e2], edges[e3]);
      gts_surface_add_face (surf, new_face);
      ta += 3;
    }
  }

  gts->ReplaceSurface(surf);
}

/**************************************************************************/
// POV export
/**************************************************************************/

#include <fstream>

void TriMesh::ExportPovMesh(const char* fname, Bool_t smoothp)
{
  static const Exc_t _eh("TriMesh::ExportPovMesh ");

  if (!mTTvor)
    throw(_eh + "mTTvor is null.");

  ofstream file(fname);
  mTTvor->ExportPovMesh(file, smoothp);
}

/**************************************************************************/
// Simple Tvors
/**************************************************************************/

void TriMesh::MakeTetrahedron(Float_t l1, Float_t l2, Float_t w, Float_t h)
{
  // l1 ~ fwd length; l2 ~ bck length; w ~ width at the end; h ~ height;

  delete mTTvor;
  mTTvor = new TringTvor(4, 4, false, true, false);
  TringTvor& C = *mTTvor;
  C.SetVertex(0, -l2, 0, -0.5*w);
  C.SetVertex(1,  l1, 0,  0);
  C.SetVertex(2, -l2, 0,  0.5*w);
  C.SetVertex(3,   0, h,  0);
  C.SetTriangle(0, 0, 1, 2);
  C.SetTriangle(1, 0, 3, 1);
  C.SetTriangle(2, 1, 3, 2);
  C.SetTriangle(3, 0, 2, 3);
  C.SetTriangleColor(0, 0,0,255);
  C.SetTriangleColor(1, 0,255,0);
  C.SetTriangleColor(2, 0,255,0);
  C.SetTriangleColor(3, 255,0,0);

  C.GenerateTriangleNormals();

  Stamp(FID());
}

/**************************************************************************/
// Vertex algorithms
/**************************************************************************/

void TriMesh::BuildVertexConnections()
{
  static const Exc_t _eh("TriMesh::BuildVertexConnections ");

  if (!mTTvor)
    throw(_eh + "mTTvor is null.");

  const Int_t nVerts = mTTvor->mNVerts;

  vector<Int_t> *trings_per_vert = new vector<Int_t> [nVerts];
  mTTvor->FindTrianglesPerVertex(trings_per_vert);

  vector<VertexData> vertex_data_vec(nVerts);
  mVDataVec.swap(vertex_data_vec);

  for (Int_t v = 0; v < nVerts; ++v)
  {
    VertexData& vdata = mVDataVec[v];

    vector<Int_t>& ts = trings_per_vert[v];
    Int_t     ts_size = (Int_t) ts.size();

    using namespace Opcode;

    // Vertex with loop index 'v' plays the role of v1.
    const Point& v1 = *(Point*) mTTvor->Vertex(v);

    // If one has an overall parametrization of the surface shape,
    // the derivatives with respect to major directions can be calculated.
    // Ideally, the principal directions can be defined as a function of
    // vertex coordinates, like u = f(x,y,z), v=g(x,y,z).
    // TriMesh is the right place to know about these.
    //
    // For now we keep them constant, as needed for x-y plane
    Point u_vec(1, 0, 0);
    Point v_vec(0, 1, 0);

    for (Int_t t = 0; t < ts_size; ++t)
    {
      Int_t vi0, vi2;  

      if (mTTvor->TriangleOtherVertices(ts[t], v, vi0, vi2) == false)
        throw (_eh + "triangle-data inconsistent.");

      const Point &v0 = mTTvor->Vertex(vi0);
      const Point &v2 = mTTvor->Vertex(vi2);

      Point e1; e1.Sub(v0, v1);
      Point e2; e2.Sub(v2, v1);

      Int_t ii0, ii2; // inner indices of VConns
      vdata.FindTwoVConnIdcs(vi0, vi2, ii0, ii2);

      VConnData &vcd0 = vdata.fVConns[ii0];
      if (vcd0.fDistance == 0)
      {
        ++vdata.fNeighbourConns;
        vcd0.fDistance = e1.Magnitude();
        vcd0.fdU       = u_vec | e1;
        vcd0.fdV       = v_vec | e1;
        vcd0.fAngle    = atan2f(vcd0.fdV, vcd0.fdU);
        if (vcd0.fAngle < 0)
          vcd0.fAngle += TMath::TwoPi();
      }

      VConnData &vcd2 = vdata.fVConns[ii2];
      if (vcd2.fDistance == 0)
      {
        ++vdata.fNeighbourConns;
        vcd2.fDistance = e2.Magnitude();
        vcd2.fdU       = u_vec | e2;
        vcd2.fdV       = v_vec | e2;
        vcd2.fAngle    = atan2f(vcd2.fdV, vcd2.fdU);
        if (vcd2.fAngle < 0)
          vcd2.fAngle += TMath::TwoPi();
      }

      // surface per half-edge ~ 1/6 * 1/2 parallelogram-surface
      Float_t surf  = 0.5f * (e1 ^ e2).Magnitude() / 6.0f;
      Float_t sprd  = 0.5f * acosf((e1 | e2) / (vcd0.fDistance * vcd2.fDistance));

      //printf("--- v=%d t=%d ts[t]=%d v0=%d v2=%d, surf=%5.2f, sprd=%5.2f\n",
      //       v, t, ts[t], vi0, vi2, surf, sprd);

      vcd0.fSpread  += sprd;
      vcd0.fSurface += surf;
      vcd2.fSpread  += sprd;
      vcd2.fSurface += surf;

      vdata.fSpread  += 2.0f * sprd;
      vdata.fSurface += 2.0f * surf;

      // Angle or dU,dV could be calculated given to two reference directions.
      // This depends on the client, really.
      // External callback?
      // This angle would also be used as a sorting criteria.
    }

    // Sort VConns according to fAngle (default operator<()).
    sort(vdata.fVConns.begin(), vdata.fVConns.end());

    /*
    printf("Vertex %2d (%5.2f, %5.2f, %5.2f) sprd=%5.3f surf=%5.3f, neighb-conns=%d, size=%d\n",
           v, v1.x, v1.y, v1.z,
           vdata.fSpread, vdata.fSurface,
           vdata.fNeighbourConns, vdata.fVConns.size());
    for (Int_t n = 0; n < vdata.fNeighbourConns; ++n)
    {
      VConnData& d = vdata.fVConns[n];

      printf("  %d  nv=%2d  d=%5.2f, sp=%5.2f, su=%5.2f, angl=%5.2f du=%5.2f, dv=%5.2f\n",
             n, d.fVTarget, d.fDistance, d.fSpread, d.fSurface, d.fAngle, d.fdU, d.fdV);
    }
    */
  }

  delete [] trings_per_vert;
}

void TriMesh::AssertVertexConnections()
{
  static const Exc_t _eh("TriMesh::AssertVertexConnections ");

  if (!mTTvor)
    throw(_eh + "mTTvor is null.");

  if ((Int_t) mVDataVec.size() !=  mTTvor->mNVerts)
    BuildVertexConnections();
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

TriMesh::VConnData& TriMesh::VertexData::FindVConn(Int_t vtarget)
{
  vector<VConnData>::iterator i = fVConns.begin();
  while (i != fVConns.end())
  {
    if (i->fVTarget == vtarget) return *i;
    ++i;
  }  
  fVConns.push_back(VConnData(vtarget));
  return fVConns.back();
}

void TriMesh::VertexData::FindTwoVConnIdcs(Int_t  v1,  Int_t  v2,
                                           Int_t& vi1, Int_t& vi2)
{
  Int_t idx = 0;
  vi1 = vi2 = -1;
  vector<VConnData>::iterator i = fVConns.begin();
  while (i != fVConns.end())
  {
    if (i->fVTarget == v1) vi1 = idx;
    if (i->fVTarget == v2) vi2 = idx;
    ++i; ++idx;
  }
  if (vi1 == -1) { fVConns.push_back(VConnData(v1)); vi1 = idx++; }
  if (vi2 == -1) { fVConns.push_back(VConnData(v2)); vi2 = idx++; }
}
