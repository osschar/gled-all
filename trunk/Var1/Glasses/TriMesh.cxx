// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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
#include <Glasses/ParaSurf.h>
#include <Glasses/RectTerrain.h>
#include <Glasses/GTSurf.h>
#include <Glasses/ZImage.h>
#include "TriMesh.c7"

#include <GTS/GTS.h>

#include <Opcode/Opcode.h>

#include <fstream>

ClassImp(TriMesh)

/**************************************************************************/

void TriMesh::_init()
{
  mTTvor     = 0;
  mVolume    = 0;
  mXYArea    = 0;

  mM    = 0;
  mSurf = 0;

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

/******************************************************************************/

namespace
{
inline void add_Js(HPointF& J, HPointF& ND, const Opcode::Point& r, Float_t m)
{
  const Float_t xs = r.x*r.x, ys = r.y*r.y, zs = r.z*r.z;
  J.x += m * (ys + zs);  ND.x -= m * r.y*r.z;
  J.y += m * (zs + xs);  ND.y -= m * r.x*r.z;
  J.z += m * (xs + ys);  ND.z -= m * r.x*r.y;
}
}

// For diagonalization of inertia tensor
// #include <TVectorT.h>
// #include <TMatrixTSym.h>

void TriMesh::SetMassAndSpeculate(Float_t mass, Float_t mass_frac_on_mesh)
{
  // Set mass of the mesh and calculate center of mass and Jz (should
  // extend for full matrix but let's keep it simple for now) with a
  // simple algorithm: distribute mass_frac_on_mesh of mass over the
  // triangles of the mesh.
  //
  // As J_spehere = 2/5 m*r^2, the default value for mass_frac_on_mesh = 0.4.

  // !!! Doubles should be used for all accumulators when adding up
  // quantites over vertices/triangles.

  mM    = mass;
  mSurf = 0;
  mCOM.Set(0, 0, 0);
  mJ  .Set(0, 0, 0);

  const Int_t nTrings = mTTvor->mNTrings;

  std::vector<Float_t> tri_surfs(nTrings);

  mSurf = 0;

  Int_t *vts;
  { // Sum surface
    Opcode::Point *p0, e1, e2, cp;
    for (Int_t tring = 0; tring < nTrings; ++tring)
    {
      vts = mTTvor->Triangle(tring);

      using namespace Opcode;
      p0 = (Point*) mTTvor->Vertex(vts[0]);
      e1.Set(mTTvor->Vertex(vts[1])); e1.Sub(*p0);
      e2.Set(mTTvor->Vertex(vts[2])); e2.Sub(*p0);
      cp.Cross(e1, e2);

      tri_surfs[tring] = 0.5f * cp.Magnitude();
      mSurf += tri_surfs[tring];
    }
  }
  Opcode::Point com(0, 0, 0);
  {
    Float_t oneoso3 = INV3 / mSurf; // one over surface over 3
    Float_t surfptv;                // surface per triangle vertex
    for (Int_t tring = 0; tring < nTrings; ++tring)
    {
      vts     = mTTvor->Triangle(tring);
      surfptv = oneoso3 * tri_surfs[tring];

      using namespace Opcode;
      com.TMac(* ((Point*) mTTvor->Vertex(vts[0])), surfptv);
      com.TMac(* ((Point*) mTTvor->Vertex(vts[1])), surfptv);
      com.TMac(* ((Point*) mTTvor->Vertex(vts[2])), surfptv);
    }
    mCOM.Set(com);
  }
  {
    // Calculate components of inertia tensor.
    // Should:
    // 1. diagonalize
    // 2. setup mesh-to-extendio transform
    //    extendio specific, once special loads are allowed

    HPointF I, J; // diagonal, off-diagonal elements

    Float_t mpso3 = INV3 * mass_frac_on_mesh * mass / mSurf; // mass per surface over 3
    Float_t mptv; // mass per triangle vertex
    Opcode::Point r;
    for (Int_t tring = 0; tring < nTrings; ++tring)
    {
      vts  = mTTvor->Triangle(tring);
      mptv = mpso3 * tri_surfs[tring];

      r.Set(mTTvor->Vertex(vts[0])); r.Sub(com); add_Js(I, J, r, mptv);
      r.Set(mTTvor->Vertex(vts[1])); r.Sub(com); add_Js(I, J, r, mptv);
      r.Set(mTTvor->Vertex(vts[2])); r.Sub(com); add_Js(I, J, r, mptv);
    }

    mJ = I;

    /*
    printf("TriMesh %s:\n", GetName());
    printf("    Ixx=%.3f Iyy=%.3f Izz=%.3f\n", I.x, I.y, I.z);
    printf("    Iyz=%.3f Ixz=%.3f Ixy=%.3f\n", J.x, J.y, J.z);

    TMatrixTSym<float> m(3);
    m(0,0) = I.x; m(0,1) = m(1,0) = J.z; m(0,2) = m(2,0) = J.y;
    m(1,1) = I.y; m(1,2) = m(2,1) = J.x;
    m(2,2) = I.z;
    m.Print();
    TVectorT<float> eval;
    TMatrixT<float> evec = m.EigenVectors(eval);
    printf("Eigen vals: %f, %f, %f\n", eval(0), eval(1), eval(2));
    evec.Print();
    */
  }

  Stamp(FID());
}

void TriMesh::SetMassFromBBox(Float_t sfac, Float_t hfac, Float_t density,
                              Float_t mass_frac_on_mesh)
{
  // Estimate XY-area, volume and mass from given parameters and
  // bounding-box of tring-tvor.

  mXYArea = sfac * mTTvor->BoundingBoxXYArea();
  mVolume = hfac * 2.0f * mTTvor->mCtrExtBox[5] * mXYArea;
  SetMassAndSpeculate(density * mVolume, mass_frac_on_mesh);
}

/**************************************************************************/

void TriMesh::StdSurfacePostImport()
{
  CalculateBoundingBox();
  BuildOpcStructs();
  BuildVertexConnections();
  mParaSurf->FindMinMaxFGH(this);
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
  mOPCMeshIf->SetPointers((IndexedTriangle*) mTTvor->Trings(),
                          (Point*)           mTTvor->Verts());

  OPCODECREATE OPCC;
  OPCC.mIMesh = mOPCMeshIf;

  bool status = mOPCModel->Build(OPCC);
  if ( ! status)
    throw(_eh + "Build failed for " + Identify() + ".");
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

  // Float_t *b = mTTvor->mMinMaxBox, *e = mTTvor->mCtrExtBox;
  // printf("%s %s\n"
  //        "  min = % 8.4f, % 8.4f, % 8.4f  max = % 8.4f, % 8.4f, % 8.4f\n"
  //        "  ctr = % 8.4f, % 8.4f, % 8.4f  ext = % 8.4f, % 8.4f, % 8.4f\n",
  //        _eh.Data(), Identify().Data(),
  //        b[0], b[1], b[2], b[3], b[4], b[5],
  //        e[0], e[1], e[2], e[3], e[4], e[5]);

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

void TriMesh::GenerateTriangleNormals()
{
  static const Exc_t _eh("TriMesh::GenerateTriangleNormals ");

  if (!mTTvor)
    throw(_eh + "mTTvor is null.");

  mTTvor->GenerateTriangleNormals();
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
    // Here we request vertex (smooth) and triangle (flat) sections.
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
  delete mTTvor;
  mTTvor = tt;
  mStampReqTring = Stamp(FID());
}

void TriMesh::ExportGTSurf(GTSurf* gts)
{
  static const Exc_t _eh("TriMesh::ExportGTSurf ");

  using namespace GTS;

  // Prepare edge data

  xx_edge_hash_t edge_map;
  Int_t          edge_cnt = 0;
  {
    Int_t* ta = mTTvor->Trings();
    for(Int_t t=0; t<mTTvor->mNTrings; ++t) {
      pair<xx_edge_hash_i, bool> res;
      res = edge_map.insert(make_pair(xx_edge(ta[0], ta[1]), edge_cnt));
      if(res.second) ++edge_cnt;
      res = edge_map.insert(make_pair(xx_edge(ta[1], ta[2]), edge_cnt));
      if(res.second) ++edge_cnt;
      res = edge_map.insert(make_pair(xx_edge(ta[2], ta[0]), edge_cnt));
      if(res.second) ++edge_cnt;
      ta += 3;
    }
  }
  printf("%sCounting edges => %d, map-size = %d.\n", _eh.Data(),
         edge_cnt, (int)edge_map.size());

  // Construct surface

  GtsSurface* surf = MakeDefaultSurface();

  GtsVertex **vertices = new GtsVertex*[mTTvor->mNVerts];
  {
    Float_t* va = mTTvor->Verts();
    for(Int_t v=0; v<mTTvor->mNVerts; ++v) {
      vertices[v] = gts_vertex_new(surf->vertex_class,
                                   va[0], va[1], va[2]);
      va += 3;
    }
  }

  GtsEdge ** edges = new GtsEdge*[edge_cnt];
  {
    for(xx_edge_hash_i e=edge_map.begin(); e!=edge_map.end(); ++e) {
      edges[e->second] = gts_edge_new(surf->edge_class,
                                      vertices[e->first.v1],
                                      vertices[e->first.v2]);
    }
  }

  {
    Int_t* ta = mTTvor->Trings();
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

void TriMesh::make_tetra(Int_t vo, Int_t to, Float_t l1, Float_t l2,
                         Float_t z, Float_t w, Float_t h)
{
  TringTvor& C = *mTTvor;

  C.SetVertex(vo + 0, -l2,  0.5*w, z);
  C.SetVertex(vo + 1,  l1,  0,     z);
  C.SetVertex(vo + 2, -l2, -0.5*w, z);
  C.SetVertex(vo + 3,   0,  0,     z+h);

  C.SetTriangle(to + 0, vo + 0, vo + 1, vo + 2);
  C.SetTriangle(to + 1, vo + 0, vo + 3, vo + 1);
  C.SetTriangle(to + 2, vo + 1, vo + 3, vo + 2);
  C.SetTriangle(to + 3, vo + 0, vo + 2, vo + 3);

  C.SetTriangleColor(to + 0, 0,0,255);
  C.SetTriangleColor(to + 1, 0,255,0);
  C.SetTriangleColor(to + 2, 0,255,0);
  C.SetTriangleColor(to + 3, 255,0,0);
}

void TriMesh::make_cubus(Int_t vo, Int_t to,
                         Float_t x0, Float_t y0, Float_t z0,
                         Float_t a,  Float_t b,  Float_t c,
                         UChar_t cr, UChar_t cg, UChar_t cb, UChar_t ca)
{
  TringTvor& C = *mTTvor;

  C.SetVertex(vo + 0, x0,     y0,     z0);
  C.SetVertex(vo + 1, x0,     y0 + b, z0);
  C.SetVertex(vo + 2, x0 + a, y0 + b, z0);
  C.SetVertex(vo + 3, x0 + a, y0,     z0);
  C.SetVertex(vo + 4, x0,     y0,     z0 + c);
  C.SetVertex(vo + 5, x0,     y0 + b, z0 + c);
  C.SetVertex(vo + 6, x0 + a, y0 + b, z0 + c);
  C.SetVertex(vo + 7, x0 + a, y0,     z0 + c);

  C.SetTriangle(to +  0, vo + 0, vo + 1, vo + 2); C.SetTriangleColor(to +  0, cr, cg, cb, ca);
  C.SetTriangle(to +  1, vo + 0, vo + 2, vo + 3); C.SetTriangleColor(to +  1, cr, cg, cb, ca);
  C.SetTriangle(to +  2, vo + 0, vo + 4, vo + 1); C.SetTriangleColor(to +  2, cr, cg, cb, ca);
  C.SetTriangle(to +  3, vo + 1, vo + 4, vo + 5); C.SetTriangleColor(to +  3, cr, cg, cb, ca);
  C.SetTriangle(to +  4, vo + 1, vo + 5, vo + 2); C.SetTriangleColor(to +  4, cr, cg, cb, ca);
  C.SetTriangle(to +  5, vo + 2, vo + 5, vo + 6); C.SetTriangleColor(to +  5, cr, cg, cb, ca);
  C.SetTriangle(to +  6, vo + 2, vo + 6, vo + 3); C.SetTriangleColor(to +  6, cr, cg, cb, ca);
  C.SetTriangle(to +  7, vo + 6, vo + 7, vo + 3); C.SetTriangleColor(to +  7, cr, cg, cb, ca);
  C.SetTriangle(to +  8, vo + 3, vo + 7, vo + 0); C.SetTriangleColor(to +  8, cr, cg, cb, ca);
  C.SetTriangle(to +  9, vo + 7, vo + 4, vo + 0); C.SetTriangleColor(to +  9, cr, cg, cb, ca);
  C.SetTriangle(to + 10, vo + 4, vo + 7, vo + 5); C.SetTriangleColor(to + 10, cr, cg, cb, ca);
  C.SetTriangle(to + 11, vo + 7, vo + 6, vo + 5); C.SetTriangleColor(to + 11, cr, cg, cb, ca);
}

void TriMesh::MakeTetrahedron(Float_t l1, Float_t l2, Float_t w, Float_t h)
{
  // l1 ~ fwd length; l2 ~ bck length; w ~ width at the end; h ~ height;

  delete mTTvor;
  mTTvor = new TringTvor(4, 4, false, true, false);

  make_tetra(0, 0, l1, l2, 0, w, h);

  mTTvor->GenerateTriangleNormals();

  Stamp(FID());
}

void TriMesh::MakeTetraFlyer(Float_t l1, Float_t l2, Float_t w, Float_t h,
                             Float_t wing_l1, Float_t wing_l2, Float_t wing_z,
                             Float_t wing_w,  Float_t wing_h)
{
  // l1 ~ fwd length; l2 ~ bck length; w ~ width at the end; h ~ height;

  delete mTTvor;
  mTTvor = new TringTvor(8, 8, false, true, false);

  make_tetra(0, 0, l1, l2, 0, w, h);
  make_tetra(4, 4, wing_l1, wing_l2, wing_z, wing_w, wing_h);

  mTTvor->GenerateTriangleNormals();

  Stamp(FID());
}

void TriMesh::MakeBox(Float_t a, Float_t b, Float_t c)
{
  // Create a box centered on 0 in x and y directions and going from 0
  // to c in z.

  delete mTTvor;
  mTTvor = new TringTvor(8, 12, false, true, false);

  make_cubus(0, 0, -0.5*a, -0.5*b, 0, a, b, c, 200, 200, 200, 255);

  mTTvor->GenerateTriangleNormals();

  Stamp(FID());

}


/**************************************************************************/
// Vertex algorithms
/**************************************************************************/

TriMesh::EdgeData::EdgeData() :
  fV1(-1), fV2(-1), fT1(-1), fT2(-1),
  fDistance(0), fDh(0),   fAngle(0),
  fSurface(0),  fSpr1(0), fSpr2(0)
{}

namespace
{
struct edge_sort_cmp
{
  const vector<TriMesh::EdgeData>& edvec;

  edge_sort_cmp(const vector<TriMesh::EdgeData>& ev) : edvec(ev) {}

  bool operator()(const Int_t& e1, const Int_t& e2)
  { return edvec[e1].fAngle < edvec[e2].fAngle; }
};
}

void TriMesh::BuildVertexConnections()
{
  static const Exc_t _eh("TriMesh::BuildVertexConnections ");

  if (!mTTvor) throw(_eh + "mTTvor is null.");

  const Int_t nVerts  = mTTvor->mNVerts;
  const Int_t nTrings = mTTvor->mNTrings;
  Int_t       nConns  = 0, nEdges = 0;

  vector<Int_t>  nconn_per_vertex(nVerts);
  xx_edge_hash_t edge_map;

  const Int_t vP[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };

  // Initialize edge-map, count edges and connections.
  for (Int_t tring = 0; tring < nTrings; ++tring)
  {
    Int_t* vts = mTTvor->Triangle(tring);
    for (Int_t e = 0; e < 3; ++e)
    {
      const Int_t v1 = vts[vP[e][0]], v2 = vts[vP[e][1]];
      if (edge_map.insert(make_pair(xx_edge(v1, v2), nEdges)).second)
      {
        ++nEdges;
        ++nconn_per_vertex[v1];
        ++nconn_per_vertex[v2];
        nConns += 2;
      }
    }
  }

  printf("%snEdges=%d, nConns=%d.\n", _eh.Data(), nEdges, nConns);

  // Allocate arrays, now their exact sizes are known
  vector<VertexData> vertex_data_vec(nVerts);
  mVDataVec.swap(vertex_data_vec);

  vector<TriMesh::EdgeData> edge_data_vec(nEdges);
  mEDataVec.swap(edge_data_vec);

  vector<Int_t> edge_curs_vec(nConns);
  mECursVec.swap(edge_curs_vec);


  // Loop over vertices, init edge-cursors
  {
    Int_t* edge_cur = & mECursVec[0];
    for (Int_t v=0; v<nVerts; ++v)
    {
      mVDataVec[v].fEdgeArr = edge_cur;
      edge_cur += nconn_per_vertex[v];
    }
  }

  // Loop over triangles, setup basic edge data
  {
    for (Int_t tring = 0; tring < nTrings; ++tring)
    {
      Int_t* vts = mTTvor->Triangle(tring);
      for (Int_t eti = 0; eti < 3; ++eti)
      {
        const Int_t v1 = vts[vP[eti][0]], v2 = vts[vP[eti][1]];
        const Int_t ei = edge_map.find(xx_edge(v1, v2))->second;
        EdgeData& ed = mEDataVec[ei];
        if (ed.fV1 == -1)
        {
          ed.fV1 = v1;  ed.fV2 = v2;  ed.fT1 = tring;
          mVDataVec[v1].insert_edge(ei);
          mVDataVec[v2].insert_edge(ei);
        }
        else
        {
          ed.fT2 = tring;
        }
      }
    }
  }

  // From this point on the edge_map is no longer used so that the
  // code can be easily moved to a separate function.

  // Loop over triangles, calc and accumulate edge & vertex-data
  {
    const Int_t oE[3]    = { 2, 0, 1 }; // other-edge for spread calculation

    for (Int_t tring = 0; tring < nTrings; ++tring)
    {
      using namespace Opcode;

      Int_t* vts = mTTvor->Triangle(tring);
      const Point* v[3] = { (Point*) mTTvor->Vertex(vts[0]), (Point*) mTTvor->Vertex(vts[1]), (Point*) mTTvor->Vertex(vts[2]) };
      Point e[3]; e[0].Sub(*v[1], *v[0]); e[1].Sub(*v[2], *v[1]); e[2].Sub(*v[0], *v[2]);

      Float_t tri_surfo3 = (e[0] ^ e[2]).Magnitude() / 6.0f;
      Float_t mags[3]    = { e[0].Magnitude(), e[1].Magnitude(), e[2].Magnitude() };
      Float_t spreads[3];  // half-angles per vertex
      for (Int_t i = 0; i < 3; ++i)
        spreads[i] = 0.5f * acosf(-(e[i] | e[oE[i]])/(mags[i]*mags[oE[i]]));

      for (Int_t eti = 0; eti < 3; ++eti)
      {
        const Int_t v1 = vts[vP[eti][0]], v2 = vts[vP[eti][1]];
        VertexData &vd = mVDataVec[v1];
        EdgeData&   ed = find_edge(vd, v1, v2);

        if (ed.fDistance == 0)
        {
          ed.fDistance = e[eti].Magnitude();
          Point mid_edge;
          mid_edge.Mac(*v[eti], e[eti], 0.5f);
          Point dirs[3];
          mParaSurf->pos2fghdir(mid_edge, dirs[0], dirs[1], dirs[2]);
          ed.fDh = e[eti] | dirs[2];
          ed.fAngle = atan2f(e[eti] | dirs[0], e[eti] | dirs[0]);
          if (ed.fAngle < 0)
            ed.fAngle += PI;
        }
        ed.fSurface  += tri_surfo3;
        if (ed.is_first(v1))
          ed.fSpr1 += spreads[vP[eti][0]], ed.fSpr2 += spreads[vP[eti][1]];
        else
          ed.fSpr2 += spreads[vP[eti][0]], ed.fSpr1 += spreads[vP[eti][1]];

        vd.fSurface += tri_surfo3;
        vd.fSpread  += 2.0f * spreads[eti];
      }
    }

    // For each vertex, sort edge cursors by their angle.
    for (Int_t v = 0; v < nVerts; ++v)
    {
      VertexData& vd = mVDataVec[v];
      sort(vd.fEdgeArr, vd.fEdgeArr + vd.fNEdges, edge_sort_cmp(mEDataVec));
    }
  }

}

void TriMesh::AssertVertexConnections()
{
  static const Exc_t _eh("TriMesh::AssertVertexConnections ");

  if (!mTTvor)
    throw(_eh + "mTTvor is null.");

  if ((Int_t) mVDataVec.size() !=  mTTvor->mNVerts)
    BuildVertexConnections();
}
