// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Tringula
//
// 

#include "Tringula.h"
#include <Glasses/ZHashList.h>
#include <Glasses/ZVector.h>
#include <Glasses/RectTerrain.h>
#include <Glasses/RGBAPalette.h>
#include <Stones/TringTvor.h>
#include "TriMesh.h"
#include "ParaSurf.h"
#include "Statico.h"
#include "Dynamico.h"
#include "Tringula.c7"

#include <Glasses/ZQueen.h>

#include <Opcode/Opcode.h>

#include <TMath.h>
#include <TF3.h>

ClassImp(Tringula);

/**************************************************************************/

void Tringula::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;
  
  bSmoothShade = false;
  bLightMesh   = true;

  mRayColFaces = 0;

  bRnrRay = false;
  mRayLen = 100;
  mRayPos.SetXYZ(1, 1, 10);
  mRayDir.SetXYZ(0, 0, -1);

  bRnrDynos  = bPickDynos   = true;
  bRnrBBoxes = bRnrItsLines = false;

  mEdgeRule   = ER_Stop;

  mRndGen.SetSeed(0);

  mBoxPruner = new Opcode::BipartiteBoxPruner;
  mStatosLTS = mDynosLTS = mFlyersLTS = 0;
}

/**************************************************************************/

Tringula::~Tringula()
{
  delete mRayColFaces;
  delete mBoxPruner;
}

void Tringula::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if (mStatos == 0) {
    ZHashList* l = new ZHashList("Statos", GForm("Statos of Tringula %s", GetName()));
    l->SetElementFID(Statico::FID());
    mQueen->CheckIn(l);
    SetStatos(l);
  }
  if (mDynos == 0) {
    ZHashList* l = new ZHashList("Dynos", GForm("Dynos of Tringula %s", GetName()));
    l->SetElementFID(Dynamico::FID());
    mQueen->CheckIn(l);
    SetDynos(l);
  }
  if (mFlyers == 0) {
    ZHashList* l = new ZHashList("Flyers", GForm("Flyers of Tringula %s", GetName()));
    l->SetElementFID(Dynamico::FID());
    mQueen->CheckIn(l);
    SetFlyers(l);
  }
}

/**************************************************************************/

void Tringula::ColorByCoord(Int_t axis, Float_t fac, Float_t offset)
{
  static const Exc_t _eh("Tringula::ColorByCoord ");

  // missing check mesh, mesh->tvor
  // should be detached?
  // missing locks

  if(axis < 0 || axis > 2)
    throw(_eh + "illegal axis.");

  assert_palette(_eh);

  TringTvor* TT = mMesh->GetTTvor();
  TT->AssertCols();
  TT->AssertBoundingBox();
  Float_t min = TT->mMinMaxBox[axis];
  Float_t max = TT->mMinMaxBox[axis + 3];
  Float_t dlt = max - min;
  mPalette->SetMinFlt(min);
  mPalette->SetMaxFlt(max);

  Float_t* V = TT->mVerts;
  UChar_t* C = TT->mCols;
  for (Int_t i=0; i<TT->mNVerts; ++i, V+=3, C+=4)
    mPalette->ColorFromValue(min + (V[axis]-min)*fac + dlt*offset, C);

  TT->GenerateTriangleColorsFromVertexColors();

  StampReqTring(FID());
}

void Tringula::ColorByNormal(Int_t axis, Float_t min, Float_t max)
{
  static const Exc_t _eh("Tringula::ColorByNormal ");

  // missing check mesh, mesh->tvor
  // should be detached?
  // missing locks

  if(axis < 0 || axis > 2)
    throw(_eh + "illegal axis.");

  assert_palette(_eh);

  TringTvor* TT = mMesh->GetTTvor();
  TT->AssertCols();
  TT->AssertBoundingBox();
  mPalette->SetMinFlt(min);
  mPalette->SetMaxFlt(max);

  Float_t* N = TT->mNorms;
  UChar_t* C = TT->mCols;
  for (Int_t i=0; i<TT->mNVerts; ++i, N+=3, C+=4)
    mPalette->ColorFromValue(N[axis], C);

  TT->GenerateTriangleColorsFromVertexColors();

  StampReqTring(FID());
}

/**************************************************************************/

void Tringula::ColorByCoordFormula(const Text_t* formula, Float_t min, Float_t max)
{
  static const Exc_t _eh("Tringula::ColorByCoordFormula ");

  assert_palette(_eh);

  TringTvor* TT = mMesh->GetTTvor();
  TT->AssertCols();
  TT->AssertBoundingBox();
  Float_t* bb = TT->mMinMaxBox;
  TF3 tf3(GForm("Tringula_CBCF_%d", GetSaturnID()), formula, 0, 0);
  tf3.SetRange(bb[0], bb[3], bb[1], bb[4], bb[2], bb[5]);
  mPalette->SetMinFlt(min);
  mPalette->SetMaxFlt(max);

  Float_t* V = TT->mVerts;
  UChar_t* C = TT->mCols;
  for (Int_t i=0; i<TT->mNVerts; ++i, V+=3, C+=4)
    mPalette->ColorFromValue((Float_t) tf3.Eval(V[0], V[1], V[2]), C);

  TT->GenerateTriangleColorsFromVertexColors();

  StampReqTring(FID());
}

void Tringula::ColorByNormalFormula(const Text_t* formula, Float_t min, Float_t max)
{
  static const Exc_t _eh("Tringula::ColorByNormalFormula ");

  assert_palette(_eh);

  TringTvor* TT = mMesh->GetTTvor();
  TT->AssertCols();
  mPalette->SetMinFlt(min);
  mPalette->SetMaxFlt(max);

  TF3 tf3(GForm("Tringula_CBNF_%d", GetSaturnID()), formula, 0, 0);
  tf3.SetRange(-1, 1, -1, 1, -1, 1);

  Float_t* N = TT->mNorms;
  UChar_t* C = TT->mCols;
  for (Int_t i=0; i<TT->mNVerts; ++i, N+=3, C+=4)
    mPalette->ColorFromValue((Float_t) tf3.Eval(N[0], N[1], N[2]), C);

  TT->GenerateTriangleColorsFromVertexColors();

  StampReqTring(FID());
}

/**************************************************************************/

void Tringula::get_ray_dir(Float_t* d, Float_t len)
{
  if(len == 0) len = mRayLen;
  d[0] = len * mRayDir.x();
  d[1] = len * mRayDir.y();
  d[2] = len * mRayDir.z();
}

/**************************************************************************/

void Tringula::SetRayVectors(const TVector3& pos, const TVector3& dir)
{
  mRayPos = pos;
  mRayDir = dir;
  Stamp(FID());
}

void Tringula::RayCollide()
{
  static const Exc_t _eh("Tringula::RayCollide ");

  if(mMesh == 0 || mMesh->GetOPCModel() == 0)
    throw(_eh + "Opcode model not created.");

  using namespace Opcode;

  RayCollider RC;
  RC.SetCulling(false);
  if(mRayColFaces == 0)
    mRayColFaces = new CollisionFaces;
  RC.SetDestination(mRayColFaces);

  Opcode::Ray R;
  mRayPos.GetXYZ((Float_t*)&R.mOrig.x);
  get_ray_dir((Float_t*)&R.mDir, 1);

  const char* setval = RC.ValidateSettings();
  printf("RayCollider::ValidateSettings: %s\n", setval ? setval : "OK.");

  bool status;
  status = RC.Collide(R, *mMesh->GetOPCModel());
           // default-args: const Matrix4x4* world=null, udword* cache=null);

  printf("collide status=%d, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
         status, RC.GetContactStatus(),
         RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections());

  CollisionFaces& CF = *mRayColFaces;
  printf("n faces = %d\n", CF.GetNbFaces());
  for(UInt_t f=0; f<CF.GetNbFaces(); ++f)
  {
    const CollisionFace& cf = CF.GetFaces()[f];
    printf("  %2d %6d  %10f  %10f %10f\n",
           f, cf.mFaceID, cf.mDistance, cf.mU, cf.mV);
  }
  Stamp(FID());
}

/**************************************************************************/

void Tringula::ResetCollisionStuff()
{
  mMesh->BuildOpcStructs();

  Stepper<Dynamico> stepper(*mDynos);
  while (stepper.step())
  {
    stepper->mOPCRCCache = OPC_INVALID_ID;
  }

  // mBoxPruner is not changed.
}

/**************************************************************************/

Float_t Tringula::PlaceAboveTerrain(ZTrans& trans, Float_t height, Float_t dh_fac)
{
  // Returns height to which the trans was placed.

  static const Exc_t _eh("Tringula::PlaceAboveTerrain ");

  using namespace Opcode;

  RayCollider    RC;
  RC.SetFirstContact(false);  // true to only take first hit (not closest!)
  RC.SetClosestHit(true);     // to keep the closes hit only
  CollisionFaces CF;
  RC.SetDestination(&CF);

  Point       pos;  trans.GetPos(pos);
  Opcode::Ray R;
  Float_t     ray_offset = mParaSurf->pos2hray(pos, R);

  Float_t     abs_height = height + dh_fac*mParaSurf->GetDeltaH();

  Bool_t status = RC.Collide(R, *mMesh->GetOPCModel());
  if (status && CF.GetNbFaces() == 1)
  {
    const CollisionFace& cf = CF.GetFaces()[0];
    pos.TMac(R.mDir, cf.mDistance - ray_offset - abs_height);
    trans.SetPos(pos);
  }
  else
  {
    throw(_eh + GForm("collide status=<%s>, contact=%d, nfaces=%d; nbvt=%d, nprt=%d, ni=%d\n",
                      status ? "ok" : "failed", RC.GetContactStatus(), CF.GetNbFaces(),
                      RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections()));
  }

  return abs_height;
}

/**************************************************************************/

Statico* Tringula::NewStatico(const Text_t* sname)
{
  if (sname == 0)
    sname = GForm("Statico %d", mStatos->GetSize() + 1);

  Statico* s = new Statico(sname);

  mParaSurf->origin_trans(s->ref_trans());

  place_on_terrain(s, *mDefStaMesh, false);

  mQueen->CheckIn(s);
  s->SetMesh(*mDefStaMesh);
  mStatos->Add(s);

  return s;
}

Statico* Tringula::RandomStatico(ZVector* mesh_list, Bool_t check_inside)
{
  // check_inside: make sure all corners of the bbox are within
  //   parasurf which is relevant for planar surfaces where statos might
  //   stick out.
  //   The argument is passed place_on_terrain() where it supresses the
  //   warnings about missed intersection.


  static const Exc_t _eh("Tringula::RandomStatico ");

  Statico* s = new Statico(GForm("Statico %d", mStatos->GetSize() + 1));

  TriMesh* mesh;
  if (mesh_list) {
    mesh = dynamic_cast<TriMesh*>
      (mesh_list->GetElementById(mRndGen.Integer(mesh_list->GetSize())));
  } else {
    mesh = *mDefStaMesh;
  }

  Int_t top_cnt = 0;
  // For bounding-box selection of collision candidates.
  setup_stato_pruner();
  // For mesh-mesh collision detection.
  Opcode::AABBTreeCollider collider;
  Opcode::BVTCache         cache;
  cache.Model0 = mesh->GetOPCModel();

place:
  mParaSurf->random_trans(mRndGen, s->ref_trans());
  s->ref_trans().RotateLF(1, 2, mRndGen.Uniform(0, TMath::TwoPi()));

  Bool_t place_status = place_on_terrain(s, mesh, check_inside);

  if (check_inside && ! place_status)
    goto place;

  Opcode::AABB bbox;
  mDefStaMesh->ref_opc_aabb().Rotate(s->ref_trans(), bbox);

  ++top_cnt;

  Opcode::Container hits;
  mBoxPruner->SinglePruning(hits, 0, bbox);
  for (UInt_t i = 0; i < hits.GetNbEntries(); ++i)
  {
    Statico *hit = (Statico*) mBoxPruner->GetUserData(0, hits.GetEntry(i));
    cache.Model1 = hit->get_opc_model();
    collider.Collide(cache, s->ref_trans(), hit->ref_trans());
    if (collider.GetContactStatus())
      goto place;
  }

  if (top_cnt > 999)
    printf("%sIntersection succ after %d\n", _eh.Data(), top_cnt);

  mQueen->CheckIn(s);
  s->SetMesh(mesh);
  mStatos->Add(s);

  return s;
}

Dynamico* Tringula::NewDynamico(const Text_t* dname)
{
  if (dname == 0)
    dname = GForm("Dynamico %d", mDynos->GetSize() + 1);

  Dynamico* d = new Dynamico(dname);
  d->SetMoveMode(Dynamico::MM_Crawl);

  mParaSurf->origin_trans(d->ref_trans());
  place_on_terrain(d);

  mQueen->CheckIn(d);
  d->SetMesh(*mDefDynMesh);
  mDynos->Add(d);

  return d;
}

Dynamico* Tringula::RandomDynamico(ZVector* mesh_list,
                                   Float_t v_min, Float_t v_max, Float_t w_max)
{
  Dynamico* d = new Dynamico(GForm("Dynamico %d", mDynos->GetSize() + 1));
  d->SetMoveMode(Dynamico::MM_Crawl);
  HTransF& t = d->ref_trans();

  TriMesh* mesh;
  if (mesh_list) {
    mesh = dynamic_cast<TriMesh*>
      (mesh_list->GetElementById(mRndGen.Integer(mesh_list->GetSize())));
  } else {
    mesh = *mDefDynMesh;
  }

  mParaSurf->random_trans(mRndGen, t);

  Float_t phi = mRndGen.Uniform(0, TMath::TwoPi());
  t.RotateLF(1, 2, phi);

  d->SetV(mRndGen.Uniform( v_min, v_max));
  d->SetW(mRndGen.Uniform(-w_max, w_max));

  place_on_terrain(d);

  mQueen->CheckIn(d);
  d->SetMesh(mesh);
  mDynos->Add(d);

  return d;
}

Dynamico* Tringula::RandomFlyer(Float_t v_min, Float_t v_max, Float_t w_max, Float_t h_max)
{
  Dynamico* d = new Dynamico(GForm("Flyer %d", mFlyers->GetSize() + 1));
  d->SetMoveMode(Dynamico::MM_Fly);
  HTransF& t = d->ref_trans();

  mParaSurf->random_trans(mRndGen, t);

  Float_t phi = mRndGen.Uniform(0, TMath::TwoPi());
  t.RotateLF(1, 2, phi);

  Float_t h = mRndGen.Uniform(0, h_max);
  t.MoveLF(3, mParaSurf->GetMaxH() + h);
  d->SetLevH(h); // This is a hack, honoured by Tringula when propagating flyers.

  d->SetV(mRndGen.Uniform( v_min, v_max));
  d->SetW(mRndGen.Uniform(-w_max, w_max));

  mQueen->CheckIn(d);
  d->SetMesh(*mDefFlyMesh);
  mFlyers->Add(d);

  return d;
}

/**************************************************************************/

void Tringula::DoDynoBoxPrunning(Bool_t detailed)
{
  static const Exc_t _eh("Tringula::DoDynoBoxPrunning ");

  using namespace Opcode;

  UInt_t nboxes = mDynos->GetSize() + mFlyers->GetSize();
  Dynamico    *dynarr[nboxes];
  const AABB  *bboxes[nboxes];

  {
    int n = 0;
    fill_pruning_list(*mDynos,  n, bboxes, (void**)dynarr);
    fill_pruning_list(*mFlyers, n, bboxes, (void**)dynarr);
  }

  Pairs  pairs;
  Axes   axes(AXES_XZY); // somewhat random
  GTime  time(GTime::I_Now);
  Bool_t res = CompleteBoxPruning(nboxes, bboxes, pairs, axes);

  printf("Box-o-pruno on %3u: res=%d, npairs=%3u, time=%f\n",
         nboxes, res, pairs.GetNbPairs(), time.TimeUntilNow().ToDouble());
  /*
  for (UInt_t i = 0; i < pairs.GetNbPairs(); ++i)
  {
      const Pair *p  = pairs.GetPair(i);
      Dynamico   *d0 = dynarr[p->id0];
      Dynamico   *d1 = dynarr[p->id1];
      printf("    %2d %s, %s\n", i+1, d0->Identify().Data(), d1->Identify().Data());
  }
  */

  if (detailed)
  {
    AABBTreeCollider collider;
    
    mItsLinesIdx = 0;

    for (UInt_t i = 0; i < pairs.GetNbPairs(); ++i)
    {
      const Pair *p  = pairs.GetPair(i);
      Dynamico   *d0 = dynarr[p->id0];
      Dynamico   *d1 = dynarr[p->id1];

      BVTCache cache;
      cache.Model0 = d0->get_opc_model();
      cache.Model1 = d1->get_opc_model();

      Bool_t s0 = collider.Collide(cache, d0->ref_trans(), d1->ref_trans());
      Bool_t s1 = collider.GetContactStatus();
      UInt_t np = collider.GetNbPairs();

      printf("  %3u: %-15s .vs. %-15s; %d, %d, %u\n", i,
             d0->GetName(), d1->GetName(), s0, s1, np);

      mItsLines.reserve(mItsLinesIdx + 3*2*np);

      // Triangle pairs
      const Pair* ps = collider.GetPairs();
      for (UInt_t j = 0; j < np; ++j)
      {
        printf("    %2u: %3d %3d\n", j, ps[j].id0, ps[j].id1);

        TringTvor *TT0 = d0->get_tring_tvor();
        TringTvor *TT1 = d1->get_tring_tvor();

        Int_t* T0 = TT0->Triangle(ps[j].id0);
        Int_t* T1 = TT1->Triangle(ps[j].id1);

        // Transform triangle vertices to my cs.
        // [ If we ever get proper dynamics, transform also velocities.
        //   Or maybe transform the intersection lines or whatever. ]
        Point V0[3], V1[3];
        for (int k=0; k<3; ++k)
        {
          d0->mTrans.MultiplyVec3(TT0->Vertex(T0[k]), 1.0f, V0[k]);
          d1->mTrans.MultiplyVec3(TT1->Vertex(T1[k]), 1.0f, V1[k]);
        }

        // Define plane of triangle T0
        Plane P(V0[0], V0[1], V0[2]);

        // Calculate distance of T1's vertices to plane P
        Float_t dst[3];
        dst[0] = P.Distance(V1[0]);
        dst[1] = P.Distance(V1[1]);
        dst[2] = P.Distance(V1[2]);

        // Determine edges of T1 that intersect plane P
        // Calculate intersection points
        Point ip[2];
        Int_t pat =  4*(dst[2] > 0) + 2*(dst[1] > 0) + (dst[0] > 0);
        printf("       dists = %f, %f, %f; pat = %d\n", dst[0], dst[1], dst[2], pat);
        switch (pat)
        {
          // void intersection_point(const Plane& P, const Point& a, const Point&b, Point& result)
          case 1:
            ip[0].Mac(V1[0], V1[1]-V1[0], dst[0] / (dst[0] - dst[1]));
            ip[1].Mac(V1[0], V1[2]-V1[0], dst[0] / (dst[0] - dst[2]));
            break;
          case 6: 
            ip[0].Mac(V1[1], V1[0]-V1[1], dst[1] / (dst[1] - dst[0]));
            ip[1].Mac(V1[2], V1[0]-V1[2], dst[2] / (dst[2] - dst[0]));
            break;
          case 2:
            ip[0].Mac(V1[1], V1[0]-V1[1], dst[1] / (dst[1] - dst[0]));
            ip[1].Mac(V1[1], V1[2]-V1[1], dst[1] / (dst[1] - dst[2]));
            break;
          case 5:
            ip[0].Mac(V1[0], V1[1]-V1[0], dst[0] / (dst[0] - dst[1]));
            ip[1].Mac(V1[2], V1[1]-V1[2], dst[2] / (dst[2] - dst[1]));
            break;
          case 3:
            ip[0].Mac(V1[1], V1[2]-V1[1], dst[1] / (dst[1] - dst[2]));
            ip[1].Mac(V1[0], V1[2]-V1[0], dst[0] / (dst[0] - dst[2]));
            break;
          case 4:
            ip[0].Mac(V1[2], V1[1]-V1[2], dst[2] / (dst[2] - dst[1]));
            ip[1].Mac(V1[2], V1[0]-V1[2], dst[2] / (dst[2] - dst[0]));
            break;
          default:
            continue;
        }

        // Clip line into triangle T0: 3 steps

        // Define triangle coords
        Point e1; e1.Sub(V0[1], V0[0]);
        Point e2; e2.Sub(V0[2], V0[0]);
        Float_t e1sq = e1.SquareMagnitude();
        Float_t e2sq = e2.SquareMagnitude();
        Float_t d    = e1 | e2;

        // Calculate u,v coords of both points.
        Point uv[2];
        for (int k=0; k<2; ++k)
        {
          Point p; p.Sub(ip[k], V0[0]);
          Float_t e1p  = e1 | p;
          Float_t e2p  = e2 | p;
          uv[k].x = (e1p * e2sq - e2p * d) / (e1sq * e2sq - d * d);
          uv[k].y = (e2p - uv[k].x * d) / e2sq;
          // uv[k].z = 0; // if this becomes relevant
        }
        Point duv; duv.Sub(uv[1], uv[0]);
        printf("       uv0 = % 5.3f, % 5.3f; uv1 = % 5.3f, % 5.3f; duv = % 5.3f, % 5.3f\n",
               uv[0].x, uv[0].y, uv[1].x, uv[1].y, duv.x, duv.y);

        // Check if outside triangle limits
        Float_t t[3];    // intersection times holder
        Int_t   ti, ts;  // time index; selected (maximal) index
        // First point ... calculate intersection multipliers
        ti = ts = 0;
        if (uv[0].x < 0)
        {
          t[ti] = -uv[0].x / duv.x;
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        if (uv[0].y < 0 )
        {
          t[ti] = -uv[0].y / duv.y;
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        if (uv[0].x+uv[0].y > 1)
        {
          t[ti] = (1 - uv[0].x - uv[0].y) / (duv.x + duv.y);
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        // First point ... fix if necessary; assume positive 't[ts]'!
        if (ti > 0)
        {
          if (ti >= 2)
          {
            if (ti == 3)
            {
              printf("       warning, all 3 conditions true! Ignoring last solution.\n");
            }
            if (t[1] > t[0]) ts = 1;
          }
          ip[0].TMac2(e1, t[ts]*duv.x, e2, t[ts]*duv.y);
          printf("       first pnt: ti=%d, t0=%f, t1=%f, ts=%d\n", ti, t[0], t[1], ts);
        }
        // Second point ... calculate intersection multipliers
        // Invert du, dv and at the end also sign of t[ts]
        ti = ts = 0;
        if (uv[1].x < 0)
        {
          t[ti] = uv[1].x / duv.x;
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        if (uv[1].y < 0 )
        {
          t[ti] = uv[1].y / duv.y;
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        if (uv[1].x+uv[1].y > 1)
        {
          t[ti] = - (1 - uv[1].x - uv[1].y) / (duv.x + duv.y);
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        // Second point ... fix if necessary; assume positive 't[ts]'!
        if (ti > 0)
        {
          if (ti >= 2)
          {
            if (ti == 3)
            {
              printf("       warning, all 3 conditions true! Ignoring last solution.\n");
            }
            if (t[1] > t[0]) ts = 1;
          }
          ip[1].TMac2(e1, -t[ts]*duv.x, e2, -t[ts]*duv.y);
          printf("       secnd pnt: ti=%d, t0=%f, t1=%f, ts=%d\n", ti, t[0], t[1], ts);
        }

        // Copy over for renderer
        mItsLines[mItsLinesIdx++] = ip[0].x;
        mItsLines[mItsLinesIdx++] = ip[0].y;
        mItsLines[mItsLinesIdx++] = ip[0].z;
        mItsLines[mItsLinesIdx++] = ip[1].x;
        mItsLines[mItsLinesIdx++] = ip[1].y;
        mItsLines[mItsLinesIdx++] = ip[1].z;
      }

    }
  }
}

void Tringula::DoFullBoxPrunning()
{
  static const Exc_t _eh("Tringula::DoFullBoxPrunning ");

  using namespace Opcode;

  GTime  time(GTime::I_Now);

  UInt_t nboxes = mStatos->GetSize() + mDynos->GetSize() + mFlyers->GetSize();
  Extendio    *extarr[nboxes];
  const AABB  *bboxes[nboxes];

  {
    int n = 0;
    fill_pruning_list(*mStatos, n, bboxes, (void**) extarr);
    fill_pruning_list(*mDynos,  n, bboxes, (void**) extarr);
    fill_pruning_list(*mFlyers, n, bboxes, (void**) extarr);
  }

  // printf("Box-o-pruno, fill took %fs\n", time.TimeUntilNow().ToDouble());

  Pairs  pairs;
  Axes   axes(AXES_XZY); // somewhat random
  Bool_t res = CompleteBoxPruning(nboxes, bboxes, pairs, axes);

  printf("Box-o-pruno on %3u: res=%d, npairs=%3u, time=%f\n",
         nboxes, res, pairs.GetNbPairs(), time.TimeUntilNow().ToDouble());
  /*
  for (UInt_t i = 0; i < pairs.GetNbPairs(); ++i)
  {
      const Pair *p  = pairs.GetPair(i);
      Extendio   *d0 = extarr[p->id0];
      Extendio   *d1 = extarr[p->id1];
      printf("    %2d %s, %s\n", i+1, d0->Identify().Data(), d1->Identify().Data());
  }
  */

  Bool_t detailed = false;
  if (detailed)
  {
    AABBTreeCollider collider;
    
    mItsLinesIdx = 0;

    for (UInt_t i = 0; i < pairs.GetNbPairs(); ++i)
    {
      const Pair *p  = pairs.GetPair(i);
      Extendio   *d0 = extarr[p->id0];
      Extendio   *d1 = extarr[p->id1];

      BVTCache cache;
      cache.Model0 = d0->get_opc_model();
      cache.Model1 = d1->get_opc_model();

      Bool_t s0 = collider.Collide(cache, d0->ref_trans(), d1->ref_trans());
      Bool_t s1 = collider.GetContactStatus();
      UInt_t np = collider.GetNbPairs();

      printf("  %3u: %-15s .vs. %-15s; %d, %d, %u\n", i,
             d0->GetName(), d1->GetName(), s0, s1, np);

      mItsLines.reserve(mItsLinesIdx + 3*2*np);

      // Triangle pairs
      const Pair* ps = collider.GetPairs();
      for (UInt_t j = 0; j < np; ++j)
      {
        printf("    %2u: %3d %3d\n", j, ps[j].id0, ps[j].id1);

        TringTvor *TT0 = d0->get_tring_tvor();
        TringTvor *TT1 = d1->get_tring_tvor();

        Int_t* T0 = TT0->Triangle(ps[j].id0);
        Int_t* T1 = TT1->Triangle(ps[j].id1);

        // Transform triangle vertices to my cs.
        // [ If we ever get proper dynamics, transform also velocities.
        //   Or maybe transform the intersection lines or whatever. ]
        Point V0[3], V1[3];
        for (int k=0; k<3; ++k)
        {
          d0->mTrans.MultiplyVec3(TT0->Vertex(T0[k]), 1.0f, V0[k]);
          d1->mTrans.MultiplyVec3(TT1->Vertex(T1[k]), 1.0f, V1[k]);
        }

        // Define plane of triangle T0
        Plane P(V0[0], V0[1], V0[2]);

        // Calculate distance of T1's vertices to plane P
        Float_t dst[3];
        dst[0] = P.Distance(V1[0]);
        dst[1] = P.Distance(V1[1]);
        dst[2] = P.Distance(V1[2]);

        // Determine edges of T1 that intersect plane P
        // Calculate intersection points
        Point ip[2];
        Int_t pat =  4*(dst[2] > 0) + 2*(dst[1] > 0) + (dst[0] > 0);
        printf("       dists = %f, %f, %f; pat = %d\n", dst[0], dst[1], dst[2], pat);
        switch (pat)
        {
          // void intersection_point(const Plane& P, const Point& a, const Point&b, Point& result)
          case 1:
            ip[0].Mac(V1[0], V1[1]-V1[0], dst[0] / (dst[0] - dst[1]));
            ip[1].Mac(V1[0], V1[2]-V1[0], dst[0] / (dst[0] - dst[2]));
            break;
          case 6: 
            ip[0].Mac(V1[1], V1[0]-V1[1], dst[1] / (dst[1] - dst[0]));
            ip[1].Mac(V1[2], V1[0]-V1[2], dst[2] / (dst[2] - dst[0]));
            break;
          case 2:
            ip[0].Mac(V1[1], V1[0]-V1[1], dst[1] / (dst[1] - dst[0]));
            ip[1].Mac(V1[1], V1[2]-V1[1], dst[1] / (dst[1] - dst[2]));
            break;
          case 5:
            ip[0].Mac(V1[0], V1[1]-V1[0], dst[0] / (dst[0] - dst[1]));
            ip[1].Mac(V1[2], V1[1]-V1[2], dst[2] / (dst[2] - dst[1]));
            break;
          case 3:
            ip[0].Mac(V1[1], V1[2]-V1[1], dst[1] / (dst[1] - dst[2]));
            ip[1].Mac(V1[0], V1[2]-V1[0], dst[0] / (dst[0] - dst[2]));
            break;
          case 4:
            ip[0].Mac(V1[2], V1[1]-V1[2], dst[2] / (dst[2] - dst[1]));
            ip[1].Mac(V1[2], V1[0]-V1[2], dst[2] / (dst[2] - dst[0]));
            break;
          default:
            continue;
        }

        // Clip line into triangle T0: 3 steps

        // Define triangle coords
        Point e1; e1.Sub(V0[1], V0[0]);
        Point e2; e2.Sub(V0[2], V0[0]);
        Float_t e1sq = e1.SquareMagnitude();
        Float_t e2sq = e2.SquareMagnitude();
        Float_t d    = e1 | e2;

        // Calculate u,v coords of both points.
        Point uv[2];
        for (int k=0; k<2; ++k)
        {
          Point p; p.Sub(ip[k], V0[0]);
          Float_t e1p  = e1 | p;
          Float_t e2p  = e2 | p;
          uv[k].x = (e1p * e2sq - e2p * d) / (e1sq * e2sq - d * d);
          uv[k].y = (e2p - uv[k].x * d) / e2sq;
          // uv[k].z = 0; // if this becomes relevant
        }
        Point duv; duv.Sub(uv[1], uv[0]);
        printf("       uv0 = % 5.3f, % 5.3f; uv1 = % 5.3f, % 5.3f; duv = % 5.3f, % 5.3f\n",
               uv[0].x, uv[0].y, uv[1].x, uv[1].y, duv.x, duv.y);

        // Check if outside triangle limits
        Float_t t[3];    // intersection times holder
        Int_t   ti, ts;  // time index; selected (maximal) index
        // First point ... calculate intersection multipliers
        ti = ts = 0;
        if (uv[0].x < 0)
        {
          t[ti] = -uv[0].x / duv.x;
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        if (uv[0].y < 0 )
        {
          t[ti] = -uv[0].y / duv.y;
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        if (uv[0].x+uv[0].y > 1)
        {
          t[ti] = (1 - uv[0].x - uv[0].y) / (duv.x + duv.y);
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        // First point ... fix if necessary; assume positive 't[ts]'!
        if (ti > 0)
        {
          if (ti >= 2)
          {
            if (ti == 3)
            {
              printf("       warning, all 3 conditions true! Ignoring last solution.\n");
            }
            if (t[1] > t[0]) ts = 1;
          }
          ip[0].TMac2(e1, t[ts]*duv.x, e2, t[ts]*duv.y);
          printf("       first pnt: ti=%d, t0=%f, t1=%f, ts=%d\n", ti, t[0], t[1], ts);
        }
        // Second point ... calculate intersection multipliers
        // Invert du, dv and at the end also sign of t[ts]
        ti = ts = 0;
        if (uv[1].x < 0)
        {
          t[ti] = uv[1].x / duv.x;
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        if (uv[1].y < 0 )
        {
          t[ti] = uv[1].y / duv.y;
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        if (uv[1].x+uv[1].y > 1)
        {
          t[ti] = - (1 - uv[1].x - uv[1].y) / (duv.x + duv.y);
          if (t[ti] > 0 && t[ti] < 1) ++ti;
        }
        // Second point ... fix if necessary; assume positive 't[ts]'!
        if (ti > 0)
        {
          if (ti >= 2)
          {
            if (ti == 3)
            {
              printf("       warning, all 3 conditions true! Ignoring last solution.\n");
            }
            if (t[1] > t[0]) ts = 1;
          }
          ip[1].TMac2(e1, -t[ts]*duv.x, e2, -t[ts]*duv.y);
          printf("       secnd pnt: ti=%d, t0=%f, t1=%f, ts=%d\n", ti, t[0], t[1], ts);
        }

        // Copy over for renderer
        mItsLines[mItsLinesIdx++] = ip[0].x;
        mItsLines[mItsLinesIdx++] = ip[0].y;
        mItsLines[mItsLinesIdx++] = ip[0].z;
        mItsLines[mItsLinesIdx++] = ip[1].x;
        mItsLines[mItsLinesIdx++] = ip[1].y;
        mItsLines[mItsLinesIdx++] = ip[1].z;
      }

    }
  }
}

void Tringula::DoSplitBoxPrunning()
{
  static const Exc_t _eh("Tringula::DoSplitBoxPrunning ");

  using namespace Opcode;

  GTime  time(GTime::I_Now);

  setup_box_pruner();

  Pairs ds_pairs;
  mBoxPruner->BipartitePruning(ds_pairs, 0, 1);

  Pairs dd_pairs;
  mBoxPruner->CompletePruning(dd_pairs, 1);

  printf("Box-o-pruno on %d/%d: pairs=%d (%d/%d), time=%f\n",
         mBoxPruner->ListSize(0), mBoxPruner->ListSize(1),
         ds_pairs.GetNbPairs() + dd_pairs.GetNbPairs(),
         ds_pairs.GetNbPairs(),  dd_pairs.GetNbPairs(),
         time.TimeUntilNow().ToDouble());
  /*
  for (UInt_t i = 0; i < ds_pairs.GetNbPairs(); ++i)
  {
      const Pair *p  = ds_pairs.GetPair(i);
      Statico   *d0 = (Statico*)  mBoxPruner->GetUserData(0, p->id0);
      Dynamico  *d1 = (Dynamico*) mBoxPruner->GetUserData(1, p->id1);
      printf("    %2d %s, %s\n", i+1, d0->Identify().Data(), d1->Identify().Data());
  }
  for (UInt_t i = 0; i < dd_pairs.GetNbPairs(); ++i)
  {
      const Pair *p  = dd_pairs.GetPair(i);
      Dynamico  *d0 = (Dynamico*) mBoxPruner->GetUserData(1, p->id0);
      Dynamico  *d1 = (Dynamico*) mBoxPruner->GetUserData(1, p->id1);
      printf("    %2d %s, %s\n", i+1, d0->Identify().Data(), d1->Identify().Data());
  }
  */
  //printf("Box-o-pruno on %3u: res=%d, npairs=%3u, time=%f\n",
  //       nboxes, res, pairs.GetNbPairs(), time.TimeUntilNow().ToDouble());
}

/**************************************************************************/

void Tringula::TimeTick(Double_t t, Double_t dt)
{
  GLensWriteHolder wlck(this);

  using namespace Opcode;

  //UInt_t nboxes = mDynos->GetSize(), boxcount = 0;
  //const AABB  *bboxes[nboxes];

  RayCollider    RC;
  RC.SetFirstContact(true);
  RC.SetTemporalCoherence(true);
  CollisionFaces CF;
  RC.SetDestination(&CF);

  Stepper<Dynamico> dyno_stepper(*mDynos);
  //if (false)
  while (dyno_stepper.step())
  {
    Dynamico& D = **dyno_stepper;

    if (D.mV != 0 || D.mW != 0)
    {
      Point old_pos, pos; // These should both be within a dynamico.

      D.mTrans.GetPos(old_pos);
      D.mTrans.MoveLF(1, dt * D.mV);
      D.mTrans.RotateLF(1, 2, dt*D.mW);
      D.mTrans.GetPos(pos);
      D.touch_aabb();

      for (Int_t p=0; p<mParaSurf->n_edge_planes(); ++p)
      {
        Float_t d = mParaSurf->edge_planes()[p].Distance(pos);
        if (d > 0) handle_edge_crossing(D, old_pos, pos, p, d);
      }

      Opcode::Ray R;
      Float_t ray_offset = mParaSurf->pos2hray(pos, R);

      UInt_t cache = D.mOPCRCCache;
      if ( RC.Collide(R, *mMesh->GetOPCModel(), 0, &D.mOPCRCCache) )
      {
        if (CF.GetNbFaces() == 1)
        {
          const CollisionFace& cf = CF.GetFaces()[0];
          pos.TMac(R.mDir, cf.mDistance - ray_offset - D.mLevH);

          if (cache != D.mOPCRCCache)
          {
            Float_t* n = mMesh->GetTTvor()->TriangleNormal(D.mOPCRCCache);
            D.mTrans.SetBaseVec(3, n);
            D.mTrans.OrtoNorm3Column(1, 3);
            D.mTrans.SetBaseVecViaCross(2);
          }
        }
      }
      else
      {
        printf("collide status=<failed>, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
               RC.GetContactStatus(),
               RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(),
               RC.GetNbIntersections());
      }

      D.mTrans.SetPos(pos);

    } // if D moving

    // bboxes[boxcount++] = &D.ref_aabb();

  } // end while dynos


  Stepper<Dynamico> flyo_stepper(*mFlyers);
  while (flyo_stepper.step())
  {
    Dynamico& D = **flyo_stepper;

    if (D.mV != 0 || D.mW != 0)
    {
      Point old_pos, pos; // These should both be within a dynamico.

      D.mTrans.GetPos(old_pos);
      D.mTrans.MoveLF(1, dt*D.mV);
      D.mTrans.RotateLF(1, 2, dt*D.mW);
      D.mTrans.GetPos(pos);
      D.touch_aabb();

      for (Int_t p=0; p<mParaSurf->n_edge_planes(); ++p)
      {
        Float_t d = mParaSurf->edge_planes()[p].Distance(pos);
        if (d > 0) handle_edge_crossing(D, old_pos, pos, p, d);
      }

      Opcode::Ray R;
      Float_t ray_offset = mParaSurf->pos2hray(pos, R);

      UInt_t cache = D.mOPCRCCache;
      if ( RC.Collide(R, *mMesh->GetOPCModel(), 0, &D.mOPCRCCache) )
      {
        if (CF.GetNbFaces() == 1)
        {
          pos.TMac(R.mDir, - ray_offset - D.mLevH);

          if (cache != D.mOPCRCCache)
          {
            pos.TMac(R.mDir, - ray_offset - D.mLevH);

            Opcode::Point p = -R.mDir;
            p.Normalize();
            D.mTrans.SetBaseVec(3, p);
            D.mTrans.OrtoNorm3Column(1, 3);
            D.mTrans.SetBaseVecViaCross(2);
          }
        }
      }
      else
      {
        printf("collide status=<failed>, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
               RC.GetContactStatus(),
               RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(),
               RC.GetNbIntersections());
      }

      D.mTrans.SetPos(pos);

    } // if D moving

    // bboxes[boxcount++] = &D.ref_aabb();

  } // end while flyers


  //Pairs  pairs;
  //Axes   axes(AXES_XZY); // somewhat random
  //GTime  time(GTime::I_Now);
  //Bool_t res = CompleteBoxPruning(nboxes, bboxes, pairs, axes);
  //printf("Box-o-pruno on %3u: res=%d, npairs=%3u, time=%f\n",
  //       nboxes, res, pairs.GetNbPairs(), time.TimeUntilNow().ToDouble());
}

/******************************************************************************/

void Tringula::fill_pruning_list(AList* extendios, Int_t& n,
                                 const Opcode::AABB** boxes, void** user_data)
{
  Stepper<> stepper(extendios);
  while (stepper.step())
  {
    boxes[n]     = &((Extendio*)*stepper)->RefAABB();
    user_data[n] = *stepper;
    ++n;
  }
}

void Tringula::fill_pruning_list(AList* extendios, Int_t& n, Int_t l)
{
  fill_pruning_list(extendios, n, mBoxPruner->BoxList(l), mBoxPruner->UserData(l));
}

void Tringula::setup_box_pruner()
{
  // Make sure internal representation of static and dynamic objects
  // for box-pruning is ok.

  setup_stato_pruner();
  setup_dyno_pruner();
}

void Tringula::setup_stato_pruner()
{
  if (mStatosLTS < mStatos->GetListTimeStamp())
  {
    mBoxPruner->InitList(0, mStatos->Size());
    int n = 0;
    fill_pruning_list(*mStatos, n, 0);
    mStatosLTS = mStatos->GetListTimeStamp();
    mBoxPruner->Sort(0);
  }
}

void Tringula::setup_dyno_pruner()
{
  if (mDynosLTS  < mDynos ->GetListTimeStamp() ||
      mFlyersLTS < mFlyers->GetListTimeStamp())
  {
    mBoxPruner->InitList(1, mDynos->Size() + mFlyers->Size());
    int n = 0;
    fill_pruning_list(*mDynos,  n, 1);
    fill_pruning_list(*mFlyers, n, 1);
    mDynosLTS  = mDynos ->GetListTimeStamp();
    mFlyersLTS = mFlyers->GetListTimeStamp();
  }

  mBoxPruner->Sort(1);
}

/******************************************************************************/

void Tringula::handle_edge_crossing
( Dynamico& D, Opcode::Point& old_pos, Opcode::Point& pos, 
  Int_t plane, Float_t dist )
{
  using namespace Opcode;

  Plane& P = mParaSurf->edge_planes()[plane];

  switch (mEdgeRule)
  {
    case ER_Stop:
    {
      pos -= dist*P.n;
      D.mV = 0;
      D.mW = 0;
      D.Stamp(D.FID());
      break;
    }

    case ER_Hold:
    {
      Point step = pos; step -= old_pos;
      Float_t depth = fabsf(P.Distance(old_pos));
      pos = old_pos + depth*step/(depth+dist);
      break;
    }

    case ER_Wrap:
    {
      if (mParaSurf->support_wrap())
      {
        mParaSurf->wrap(pos, plane, dist);
        break;
      }
      // NO BREAK if wrap not supported, fallback to bounce.
    }

    case ER_Bounce:
    {
      pos -= 2*dist*P.n;
      // Rotate accordingly
      HTransF& t = D.ref_trans();
      Point& fwd = * (Point*) t.PtrBaseVec(1);
      fwd += -2 * (fwd | P.n) * P.n;
      t.OrtoNorm3Column(2, 1);
      t.SetBaseVecViaCross(3);
      break;
    }

  } // end switch
}

/**************************************************************************/

Bool_t Tringula::place_on_terrain(Statico* S, TriMesh* M, Bool_t check_inside)
{
  // Place statico on terrain so that the bounding box of the mesh
  // touches or penetrates the terrain. The distance is sampled on a
  // 3x3 grid.
  //
  // It is assumed that the statico is oriented along the local up
  // direction with zero height. If the trans matrix were fixed here,
  // the rotation would be somewhat poorly defined.
  //
  // If check_inside is true, the first miss of the terrain in the ray
  // collision test results in a termination of the loop over sampling
  // points. false is returned and no error message is printed.
  //
  // Otherwise true is returned.

  static const Exc_t _eh("Tringula::place_on_terrain ");

  if (M == 0) {
    M = S->GetMesh();
    if (M == 0)
      throw(_eh + "TriMesh not passed as argument nor available from Statico.\n");
  }

  HTransF&      trans = S->ref_trans();
  Opcode::AABB& aabb  = M->ref_opc_aabb();

  Float_t ray_offset = mParaSurf->GetMaxH() - aabb.GetMin(Opcode::_Z) +
    mParaSurf->GetEpsilon() +
    0.1f*(mParaSurf->GetMaxH() - mParaSurf->GetMinH()); // !! For local curvature, 1/10 of delta_h
  Float_t max_dist   = 0;

  Opcode::Point fdir  (trans.ArrX());
  Opcode::Point gdir  (trans.ArrY());
  Opcode::Point center(trans.ArrT());
  center.TMac(trans.ArrZ(), ray_offset);

  Opcode::Ray R;
  // R.mOrig set in loop
  R.mDir.Set(trans.ArrZ()); R.mDir.Neg();

  Opcode::RayCollider RC;
  RC.SetFirstContact(false);  // true to only take first hit (not closest!)
  RC.SetClosestHit(true);     // to keep the closes hit only

  Opcode::CollisionFaces CF;
  RC.SetDestination(&CF);

  static const Float_t sample[] = { -1, 0, 1 };
  static const Int_t   ns = 3;

  for (Int_t sf = 0; sf < ns; ++sf)
  {
    for (Int_t sg = 0; sg < ns; ++sg)
    {
      R.mOrig.Mac2(center,
                   fdir, sample[sf]*aabb.GetExtents(Opcode::_X),
                   gdir, sample[sg]*aabb.GetExtents(Opcode::_Y));
      Int_t cs = RC.Collide(R, *mMesh->GetOPCModel());
      if (cs && CF.GetNbFaces() == 1)
      {
        max_dist = TMath::Max(max_dist, CF.GetFaces()[0].mDistance);
      }
      else
      {
        if (check_inside && cs == 1)
          return false;
        else
          printf("%s(Statico*) sample_id %2d,%2d; status=%s, nfaces=%d\n"
                 "  nbvt=%d, nprt=%d, ni=%d\n"
                 "  ray_orig = %6.2f, %6.2f, %6.2f; ray_dir = %6.2f, %6.2f, %6.2f\n",
                 _eh.Data(), sf, sg, cs ? "ok" : "failed", CF.GetNbFaces(),
                 RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections(),
                 R.mOrig.x, R.mOrig.y, R.mOrig.z, R.mDir.x, R.mDir.y, R.mDir.z);
      }
    }
  }

  // printf("ray_offset = %f, max_dist = %f; to_move = %f\n",
  //        ray_offset, max_dist, ray_offset - max_dist);

  trans.MoveLF(3, ray_offset - max_dist);
  S->touch_aabb();

  return true;
}

void Tringula::place_on_terrain(Dynamico* D)
{
  static const Exc_t _eh("Tringula::place_on_terrain ");

  Opcode::RayCollider    RC;
  RC.SetFirstContact(false);  // true to only take first hit (not closest!)
  RC.SetClosestHit(true);     // to keep the closes hit only
  Opcode::CollisionFaces CF;
  RC.SetDestination(&CF);

  HTransF & trans = D->ref_trans();
  Opcode::Point& pos   = * (Opcode::Point*) trans.PtrPos();
  Opcode::Ray R;
  Float_t ray_offset = mParaSurf->pos2hray(pos, R);

  Int_t cs = RC.Collide(R, *mMesh->GetOPCModel());
  if (cs && CF.GetNbFaces() == 1)
  {
      const Opcode::CollisionFace& cf = CF.GetFaces()[0];
      pos.TMac(R.mDir, cf.mDistance - ray_offset - D->mLevH);

      Float_t* n = mMesh->GetTTvor()->TriangleNormal(cf.mFaceID);
      trans.SetBaseVec(3, n);
      trans.OrtoNorm3Column(1, 3);
      trans.SetBaseVecViaCross(2);
      D->touch_aabb();
  }
  else
  {
    printf("%s(Dynamico*) status=%s, nfaces=%d\n"
           "  nbvt=%d, nprt=%d, ni=%d\n"
           "  ray_orig = %6.2f, %6.2f, %6.2f; ray_dir = %6.2f, %6.2f, %6.2f\n",
           _eh.Data(), cs ? "ok" : "failed", CF.GetNbFaces(),
           RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections(),
           R.mOrig.x, R.mOrig.y, R.mOrig.z, R.mDir.x, R.mDir.y, R.mDir.z);
  }
}

/**************************************************************************/
