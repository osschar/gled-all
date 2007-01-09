// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Tringula
//
// Many functions implicitly assume rectangular ares limited by
// 4 edge-planes defined in SetEdgePlanes(RectTerrain*).

#include "Tringula.h"
#include <Glasses/ZHashList.h>
#include <Glasses/RectTerrain.h>
#include <Stones/TringTvor.h>
#include "TriMesh.h"
#include "Dynamico.h"
#include "Tringula.c7"

#include <Glasses/ZQueen.h>

#include <Opcode/Opcode.h>


ClassImp(Tringula);

/**************************************************************************/

void Tringula::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;
  
  bPreferSmooth = false;

  mOPCCFaces = 0;
  mOPCRayCol = 0;

  bRnrRay = false;
  mRayLen = 100;
  mRayPos.SetXYZ(1, 1, 10);
  mRayDir.SetXYZ(0, 0, -1);

  bRnrDynos  = bPickDynos   = true;
  bRnrBBoxes = bRnrItsLines = false;

  mNumEPlanes = 0;
  mEdgePlanes = 0;
  mEdgeRule   = ER_Stop;

  mRndGen.SetSeed(0);
}

Tringula::~Tringula()
{
  delete mOPCCFaces;
  delete mOPCRayCol;
  delete [] mEdgePlanes;
}

void Tringula::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mDynos == 0) {
    ZHashList* l = new ZHashList("Dynos", GForm("Dynos of Tringula %s", GetName()));
    l->SetElementFID(Dynamico::FID());
    mQueen->CheckIn(l);
    SetDynos(l);
  }
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

void Tringula::MakeOpcodeModel()
{
  static const Exc_t _eh("Tringula::MakeOpcodeModel ");

  if (mMesh == 0)
    throw(_eh + "mMesh is null.");

  using namespace Opcode;

  // Reset all collision caches in Dynos !!!!

  mMesh->BuildOpcStructs();

  if (mOPCRayCol == 0) {
    mOPCRayCol = new RayCollider;

    RayCollider& RC = *mOPCRayCol;
    RC.SetFirstContact(true);
    RC.SetTemporalCoherence(true);

    if(mOPCCFaces == 0)
      mOPCCFaces = new CollisionFaces;
    RC.SetDestination(mOPCCFaces);
  }
}

void Tringula::RayCollide()
{
  static const Exc_t _eh("Tringula::RayCollide ");

  if(mMesh == 0 || mMesh->GetOPCModel() == 0)
    throw(_eh + "Opcode model not created.");

  using namespace Opcode;

  RayCollider RC;
  RC.SetFirstContact(false);  // true to only take first hit (not closest!)
  RC.SetClosestHit(true);     // to sort the hits by distance
  RC.SetCulling(false);

  // primitive tests on by default

  if(mOPCCFaces == 0)
    mOPCCFaces = new CollisionFaces;
  RC.SetDestination(mOPCCFaces);

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

  CollisionFaces& CF = *mOPCCFaces;
  printf("n faces = %d\n", CF.GetNbFaces());
  for(UInt_t f=0; f<CF.GetNbFaces(); ++f) {
    const CollisionFace& cf = CF.GetFaces()[f];
    printf("  %2d %6d  %10f  %10f %10f\n",
           f, cf.mFaceID, cf.mDistance, cf.mU, cf.mV);
  }
  Stamp(FID());
}

/**************************************************************************/

Dynamico* Tringula::NewDynamico(const Text_t* dname)
{
  if (dname == 0)
    dname = GForm("Dynamico %d", mDynos->GetSize() + 1);

  Dynamico* d = new Dynamico(dname);
  place_on_terrain(*d);

  mQueen->CheckIn(d);
  d->SetMesh(*mDefDynMesh);
  mDynos->Add(d);

  return d;
}

Dynamico* Tringula::RandomDynamico(Float_t v_min, Float_t v_max, Float_t w_max)
{
  Float_t x = mRndGen.Uniform(mEdgePlanes[1].d, -mEdgePlanes[3].d);
  Float_t y = mRndGen.Uniform(mEdgePlanes[0].d, -mEdgePlanes[2].d);

  Float_t phi = mRndGen.Uniform(0, TMath::TwoPi());
  Float_t cos = TMath::Cos(phi), sin = TMath::Sin(phi);

  Dynamico* d = new Dynamico(GForm("Dynamico %d", mDynos->GetSize() + 1));
  HTransF& t = d->ref_trans();
  t.SetBaseVec(1, cos,  sin, 0);
  t.SetBaseVec(2, 0,      0, 1);
  t.SetBaseVec(3, sin, -cos, 0);
  t.SetBaseVec(4, x, y, 0);
  d->SetV(mRndGen.Uniform( v_min, v_max));
  d->SetW(mRndGen.Uniform(-w_max, w_max));

  place_on_terrain(*d);

  mQueen->CheckIn(d);
  d->SetMesh(*mDefDynMesh);
  mDynos->Add(d);

  return d;
}

/**************************************************************************/

void Tringula::SetEdgePlanes(RectTerrain* rect_terr)
{
  // Set bounding-planes for x/y coordinates.
  //
  // Assumes RectTerrain *always* stretches from 0,0 into the positive
  // direction (which is so far true, at least for TrinTvor, rendering
  // can be offset). Oh yess ... be carefull about the border,
  // too, as its role is not very well defined right now.

  static const Exc_t _eh("Tringula::SetEdgePlanes ");

  if (rect_terr == 0)
    throw(_eh + "null argument");

  if (mEdgePlanes) delete [] mEdgePlanes;
  mNumEPlanes = 4;
  mEdgePlanes = new Opcode::Plane[4];
  RectTerrain& R = *rect_terr;
  mEdgePlanes[0].Set( 0, -1, 0, 0);
  mEdgePlanes[1].Set(-1,  0, 0, 0);
  mEdgePlanes[2].Set( 0,  1, 0, - (R.GetNy() - 1) * R.GetDy());
  mEdgePlanes[3].Set( 1,  0, 0, - (R.GetNx() - 1) * R.GetDx());
}

/**************************************************************************/

void Tringula::DoBoxPrunning(Bool_t detailed)
{
  static const Exc_t _eh("Tringula::DoBoxPrunning ");

  using namespace Opcode;

  UInt_t nboxes = mDynos->GetSize();
  Dynamico    *dynarr[nboxes];
  const AABB  *bboxes[nboxes];

  {
    int n = 0;
    Stepper<> stepper(*mDynos);
    while (stepper.step())
    {
      Dynamico* D = (Dynamico*) *stepper;
      dynarr[n] = D;
      bboxes[n] = & D->ref_aabb();
      ++n;
    }
  }

  Pairs  pairs;
  Axes   axes(AXES_XZY); // somewhat random
  GTime  time(GTime::I_Now);
  Bool_t res = CompleteBoxPruning(nboxes, bboxes, pairs, axes);

  printf("Box-o-pruno on %3u: res=%d, npairs=%3u, time=%f\n",
         nboxes, res, pairs.GetNbPairs(), time.TimeUntilNow().ToDouble());

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
          ip[0].TMac2(e1, e2, t[ts]*duv.x, t[ts]*duv.y);
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
          ip[1].TMac2(e1, e2, -t[ts]*duv.x, -t[ts]*duv.y);
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

/**************************************************************************/

void Tringula::TimeTick(Double_t t, Double_t dt)
{
  GLensWriteHolder wlck(this);

  using namespace Opcode;

  //UInt_t nboxes = mDynos->GetSize(), boxcount = 0;
  //const AABB  *bboxes[nboxes];

  Stepper<Dynamico> stepper(*mDynos);
  while (stepper.step()) {

    Dynamico& D = **stepper;

    if (D.mV != 0 || D.mW != 0)
    {
      Point old_pos, pos; // These should both be within a dynamico.

      D.mTrans.GetPos(old_pos);
      D.mTrans.MoveLF(1, dt * D.mV);
      D.mTrans.RotateLF(3, 1, dt*D.mW);
      D.mTrans.GetPos(pos);
      D.touch_aabb();

      Point vertical(0, 0, -1);

      for (Int_t p=0; p<mNumEPlanes; ++p) {
        Float_t d = mEdgePlanes[p].Distance(pos);
        if (d > 0) handle_edge_crossing(D, old_pos, pos, p, d);
      }

      RayCollider&    RC = *mOPCRayCol;
      CollisionFaces& CF = *mOPCCFaces;

      static const Float_t ray_offset = 100;

      Opcode::Ray R(pos, vertical);
      R.mOrig.z += ray_offset;

      UInt_t cache = D.mOPCRCCache;
      if ( RC.Collide(R, *mMesh->GetOPCModel(), 0, &D.mOPCRCCache) ) {

        if (CF.GetNbFaces() == 1) {
          const CollisionFace& cf = CF.GetFaces()[0];
          pos.z += D.mLevH - cf.mDistance + ray_offset;

          if (cache != D.mOPCRCCache) {
            Float_t* n = mMesh->GetTTvor()->TriangleNormal(D.mOPCRCCache);
            D.mTrans.SetBaseVec(2, n);
            D.mTrans.OrtoNorm3Column(1, 2);
            D.mTrans.SetBaseVecViaCross(3);
          }
        }

      } else {

        printf("collide status=<failed>, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
               RC.GetContactStatus(),
               RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(),
               RC.GetNbIntersections());

      }

      D.mTrans.SetPos(pos);

    } // if D moving

    // bboxes[boxcount++] = &D.ref_aabb();

  } // end while dynos

  //Pairs  pairs;
  //Axes   axes(AXES_XZY); // somewhat random
  //GTime  time(GTime::I_Now);
  //Bool_t res = CompleteBoxPruning(nboxes, bboxes, pairs, axes);
  //printf("Box-o-pruno on %3u: res=%d, npairs=%3u, time=%f\n",
  //       nboxes, res, pairs.GetNbPairs(), time.TimeUntilNow().ToDouble());
}

void Tringula::handle_edge_crossing
( Dynamico& D, Opcode::Point& old_pos, Opcode::Point& pos, 
  Int_t plane, Float_t dist )
{
  using namespace Opcode;

  Plane& P = mEdgePlanes[plane];

  switch (mEdgeRule)
    {
    case ER_Stop: {
      pos -= dist*P.n;
      D.mV = 0;
      D.mW = 0;
      D.Stamp(D.FID());
      break;
    }
    case ER_Hold: {
      Point step = pos; step -= old_pos;
      Float_t depth = fabsf(P.Distance(old_pos));
      pos = old_pos + depth*step/(depth+dist);
      break;
    }
    case ER_Bounce: {
      pos -= 2*dist*P.n;
      // Rotate accordingly
      HTransF& t = D.ref_trans();
      Point& fwd = * (Point*) t.PtrBaseVec(1);
      fwd += -2 * (fwd | P.n) * P.n;
      t.OrtoNorm3Column(2, 1);
      t.SetBaseVecViaCross(3);
      break;
    }
    case ER_Wrap: {
      // Cross-over the opposite plane, assume 4 planes pre-defined
      // rectangular layout.
      Int_t opp_plane = (plane + 2) % mNumEPlanes;
      Plane& O = mEdgePlanes[opp_plane];
      Float_t opp_dist = O.Distance(pos);
      pos += dist * P.n - opp_dist * O.n;
      break;
    }

    } // end switch
}

/**************************************************************************/

void Tringula::place_on_terrain(Dynamico& D)
{
  using namespace Opcode;

  Point &pos = * (Point*) D.mTrans.PtrPos();
  Point vertical(0, 0, -1);

  RayCollider&    RC = *mOPCRayCol;
  CollisionFaces& CF = *mOPCCFaces;

  static const Float_t ray_offset = 100;

  Opcode::Ray R(pos, vertical);
  R.mOrig.z += ray_offset;

  UInt_t cache = D.mOPCRCCache;
  if ( RC.Collide(R, *mMesh->GetOPCModel(), 0, &D.mOPCRCCache) ) {

    if (CF.GetNbFaces() == 1) {
      const CollisionFace& cf = CF.GetFaces()[0];
      pos.z += D.mLevH - cf.mDistance + ray_offset;

      if (cache != D.mOPCRCCache) {
        Float_t* n = mMesh->GetTTvor()->TriangleNormal(D.mOPCRCCache);
        D.mTrans.SetBaseVec(2, n);
        D.mTrans.OrtoNorm3Column(1, 2);
        D.mTrans.SetBaseVecViaCross(3);
        D.touch_aabb();
      }
    }

  } else {

    printf("collide status=<failed>, contact=%d; nbvt=%d, nprt=%d, ni=%d\n",
           RC.GetContactStatus(),
           RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections());

  }
}

/**************************************************************************/
