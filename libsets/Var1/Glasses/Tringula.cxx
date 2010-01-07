// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Tringula.h"
#include <Glasses/ZHashList.h>
#include <Glasses/ZVector.h>
#include <Glasses/RectTerrain.h>
#include <Glasses/ZImage.h>
#include <Stones/TringTvor.h>
#include "TriMesh.h"
#include "ParaSurf.h"
#include "Statico.h"
#include "Dynamico.h"
#include "Crawler.h"
#include "Flyer.h"
#include "Airplane.h"
#include "Chopper.h"
#include "LandMark.h"

#include "Tringula.c7"

#include <Glasses/WSTube.h>

#include <Glasses/ZQueen.h>

#include <Opcode/Opcode.h>

#include <TMath.h>

//__________________________________________________________________________
// Tringula
//
//

ClassImp(Tringula);

/**************************************************************************/

void Tringula::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  // This is a temporary hack needed because the flyer dynamic is too simple.
  // It prevents flyers from flying into regions where gravity changes
  // direction abruptly.
  // There can be no reasonable default, so it is set to 0 to make
  // it rather obvious when it is not set.
  mMaxFlyerH   = 0;
  mMaxCameraH  = 0;

  mRayColFaces = 0;

  bRnrRay = false;
  mRayLen = 100;
  mRayPos.SetXYZ(1, 1, 10);
  mRayDir.SetXYZ(0, 0, -1);

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
  if (mLandMarks == 0) {
    ZHashList* l = new ZHashList("LandMarks", GForm("LandMarks of Tringula %s", GetName()));
    l->SetElementFID(LandMark::FID());
    mQueen->CheckIn(l);
    SetLandMarks(l);
  }
  if (mTubes == 0) {
    ZHashList* l = new ZHashList("Tubes", GForm("Tubes of Tringula %s", GetName()));
    l->SetElementFID(WSTube::FID());
    mQueen->CheckIn(l);
    SetTubes(l);
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

void Tringula::SetRayVectors(const TVector3& pos, const TVector3& dir)
{
  mRayPos = pos;
  mRayDir = dir;
  Stamp(FID());
}

void Tringula::RayCollide()
{
  // Intersect mesh with ray as given in data-members mRayPos and mRayDir.
  //
  // Should really take Opcode::Ray as argument.

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
  mRayPos.GetXYZ(R.mOrig);
  get_ray_dir(R.mDir, 1);

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

void Tringula::prepick_extendios(AList        * extendios,
                                 Opcode::Ray  & ray,
                                 lPickResult_t& candidates)
{
  // Select picking candiadates from among extendios.

  Opcode::Point dpos;
  Float_t       t;

  Stepper<> stepper(extendios);
  while (stepper.step())
  {
    Extendio* ext = (Extendio*) *stepper;

    dpos.Sub(ray.mOrig, ext->ref_last_aabb().Center());
    t = - (ray.mDir | dpos);

    if (t > 0 && dpos.SquareMagnitude() - t*t <= ext->ref_last_aabb().GetSphereSquareRadius())
    {
      candidates.push_back(PickResult(ext, t));
    }
  }
}

Extendio* Tringula::PickExtendios()
{
  // Loop over extendios and calculate distance between ray and center
  // point.
  //
  // Original idea was to sub-divide ray into boxes and do split box prunning.
  //
  // Could increase size of boxes further away.
  //
  // In fact, only need to calculate until the first contact with terrain.
  //
  // Check both options.

  static const Exc_t _eh("Tringula::PickExtendios ");

  Opcode::Ray ray;
  mRayPos.GetXYZ(ray.mOrig);
  get_ray_dir(ray.mDir, 1);

  lPickResult_t candidates;

  prepick_extendios(*mStatos,    ray, candidates);
  prepick_extendios(*mDynos,     ray, candidates);
  prepick_extendios(*mFlyers,    ray, candidates);
  prepick_extendios(*mLandMarks, ray, candidates);

  candidates.sort();

  Opcode::RayCollider RC;
  RC.SetFirstContact(true);

  for (lPickResult_i res = candidates.begin(); res != candidates.end(); ++res)
  {
    Extendio *ext = res->fExtendio;

    if (RC.Collide(ray, * ext->get_opc_model(), ext->RefLastTrans()))
    {
      if (RC.GetContactStatus())
      {
        return ext;
      }
    }
    else
    {
      printf("%scollide status=<failed>, extendio='%s'.", _eh.Data(), ext->GetName());
    }
  }
  return 0;
}

/**************************************************************************/

void Tringula::ResetCollisionStuff()
{
  mMesh->BuildOpcStructs();

  Stepper<Dynamico> stepper(*mDynos);
  while (stepper.step())
  {
    stepper->mOPCRCCache = Opcode::OPC_INVALID_ID;
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
    throw _eh + RC.CollideInfo(status, R);
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
  s->SetTringula(this);

  s->update_aabb();

  return s;
}

Statico* Tringula::RandomStatico(ZVector *mesh_list,
                                 Bool_t   check_inside,
                                 Int_t    max_tries)
{
  // Randomly select a mesh for statico from mesh_list and place it on
  // a random location on the terrain. Makes sure the new statico
  // does not overlap with existing ones.
  //
  // check_inside: make sure all corners of the bbox are within
  //   parasurf which is relevant for planar surfaces where statos might
  //   stick out.
  //   check_inside is also passed to place_on_terrain() where it
  //   suppresses the warnings about missed intersection.
  // max_tries: how many times to try statico's placement on terrain.
  //   If this limit is reached an exception is thrown.
  //   As CINT doesn't handle this very well, 0 is returned at the moment.
  //
  //   This is relevant when many staticos are already on the terrain
  //   and for steep terrain.
  //
  // How much of statico's bbox must stick out of terrain on each
  // sampling-point is hardcoded to 0.5*bbox_z_size.

  static const Exc_t _eh("Tringula::RandomStatico ");

  Statico* s = new Statico(GForm("Statico %d", mStatos->GetSize() + 1));

  TriMesh* mesh;
  if (mesh_list) {
    mesh = dynamic_cast<TriMesh*>
      (mesh_list->GetElementById(mRndGen.Integer(mesh_list->GetSize())));
  } else {
    mesh = *mDefStaMesh;
  }

  // For bounding-box selection of collision candidates.
  setup_stato_pruner();
  // For mesh-mesh collision detection.
  Opcode::AABB             bbox;
  Opcode::AABBTreeCollider collider;
  Opcode::BVTCache         cache;
  cache.Model0 = mesh->GetOPCModel();

  Int_t top_cnt = 0;
place:
  if (++top_cnt > max_tries)
    // throw (_eh + "max_tries reached.");
  // This is temporary solution to bypass the problem with exceptions 
  // not being deliver into interpreted code.
  // Using semi-functional solution now for further investigation.
    return 0;

  mParaSurf->random_trans(mRndGen, s->ref_trans());
  s->ref_trans().RotateLF(1, 2, mRndGen.Uniform(0, TMath::TwoPi()));

  Bool_t place_status = place_on_terrain(s, mesh, check_inside,
                                         0.5f * mesh->ref_mesh_bbox().GetZSize());

  if (check_inside && ! place_status)
    goto place;

  mesh->ref_mesh_bbox().Rotate(s->ref_trans(), bbox);

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

  mQueen->CheckIn(s);
  s->SetMesh(mesh);
  mStatos->Add(s);
  s->SetTringula(this);

  s->update_aabb();

  return s;
}

void Tringula::RegisterCrawler(Crawler* d)
{
  mParaSurf->origin_trans(d->ref_trans());
  place_on_terrain(d, d->GetLevH());

  mQueen->CheckIn(d);
  d->SetMesh(*mDefDynMesh);
  mDynos->Add(d);
  d->SetTringula(this);

  d->update_aabb();
  d->update_last_data();
}

Crawler* Tringula::NewDynamico(const Text_t* dname)
{
  if (dname == 0)
    dname = GForm("Dynamico %d", mDynos->GetSize() + 1);

  Crawler* d = new Crawler(dname);

  RegisterCrawler(d);

  return d;
}

Crawler* Tringula::RandomDynamico(ZVector* mesh_list,
                                   Float_t v_min, Float_t v_max, Float_t w_max)
{
  Crawler* d = new Crawler(GForm("Dynamico %d", mDynos->GetSize() + 1));
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

  place_on_terrain(d, d->GetLevH());

  mQueen->CheckIn(d);
  d->SetMesh(mesh);
  mDynos->Add(d);
  d->SetTringula(this);

  d->update_aabb();
  d->update_last_data();

  return d;
}

Dynamico* Tringula::RandomAirplane(Float_t v_min, Float_t v_max,
                                   Float_t w_max,
                                   Float_t h_min_fac, Float_t h_max_fac)
{
  static const Exc_t _eh("Tringula::RandomAirplane ");

  Flyer* d = new Airplane(GForm("Airplane %d", mFlyers->GetSize() + 1));
  HTransF& t = d->ref_trans();

  mParaSurf->random_trans(mRndGen, t);

  Float_t phi = mRndGen.Uniform(0, TMath::TwoPi());
  t.RotateLF(1, 2, phi);

  Float_t point_h, terrain_h;
  if ( ! terrain_height(t.ref_pos(), point_h, terrain_h))
    throw _eh + "problem determining terrain height.";

  Float_t h = mRndGen.Uniform(h_min_fac * mMaxFlyerH, h_max_fac * mMaxFlyerH);
  h = TMath::Max(h, terrain_h + mDefFlyMesh->GetTTvor()->BoundingBoxDiagonal());
  t.MoveLF(3, h);
  d->SetHeight(h);
  // printf("Set height=%+6.2f for flyer '%12s', th=%+6.2f, ph=%+6.2f, maxfh=%+6.2f; %+6.2f-%+6.2f\n",
  //        h, d->GetName(), terrain_h, point_h, mMaxFlyerH,
  //        h_min_fac * mMaxFlyerH, h_max_fac * mMaxFlyerH);

  d->SetV(mRndGen.Uniform( v_min, v_max));
  d->SetW(mRndGen.Uniform(-w_max, w_max));

  mQueen->CheckIn(d);
  d->SetMesh(*mDefFlyMesh);
  mFlyers->Add(d);
  d->SetTringula(this);

  d->update_aabb();
  d->update_last_data();

  return d;
}

Dynamico* Tringula::RandomChopper(Float_t v_min, Float_t v_max,
                                  Float_t w_max,
                                  Float_t h_min_fac, Float_t h_max_fac)
{
  static const Exc_t _eh("Tringula::RandomChopper ");

  Flyer* d = new Chopper(GForm("Chopper %d", mFlyers->GetSize() + 1));
  HTransF& t = d->ref_trans();

  mParaSurf->random_trans(mRndGen, t);

  Float_t phi = mRndGen.Uniform(0, TMath::TwoPi());
  t.RotateLF(1, 2, phi);

  Float_t point_h, terrain_h;
  if ( ! terrain_height(t.ref_pos(), point_h, terrain_h))
    throw _eh + "problem determining terrain height.";

  Float_t h = mRndGen.Uniform(h_min_fac * mMaxFlyerH, h_max_fac * mMaxFlyerH);
  h = TMath::Max(h, terrain_h + mDefChopMesh->GetTTvor()->BoundingBoxDiagonal());
  t.MoveLF(3, h);
  d->SetHeight(h);
  // printf("Set height=%+6.2f for flyer '%12s', th=%+6.2f, ph=%+6.2f, maxfh=%+6.2f; %+6.2f-%+6.2f\n",
  //        h, d->GetName(), terrain_h, point_h, mMaxFlyerH,
  //        h_min_fac * mMaxFlyerH, h_max_fac * mMaxFlyerH);

  d->SetV(mRndGen.Uniform( v_min, v_max));
  d->SetW(mRndGen.Uniform(-w_max, w_max));

  mQueen->CheckIn(d);
  d->SetMesh(*mDefChopMesh);
  mFlyers->Add(d);
  d->SetTringula(this);

  d->update_aabb();
  d->update_last_data();

  return d;
}

LandMark* Tringula::AddLandMark(TriMesh* mesh, const Float_t* pos)
{
  LandMark* d = new LandMark(GForm("LandMark %d", mLandMarks->GetSize() + 1));
  HTransF& t = d->ref_trans();

  Float_t fgh[3];
  mParaSurf->pos2fgh(pos, fgh);
  mParaSurf->fgh2trans(fgh, t);

  mQueen->CheckIn(d);
  d->SetMesh(mesh);
  mLandMarks->Add(d);
  d->SetTringula(this);
  d->SetFGH(fgh[0], fgh[1], fgh[2]);

  d->update_aabb();
  d->update_last_data();

  return d;
}

//==============================================================================

Bool_t Tringula::CheckBoundaries(Dynamico* dyno, Float_t& safety)
{
  // Checks if dyno crossed some important boundary and handles that.
  // This means that Dyno's velocity and transformation matrix might be
  // modified.
  //
  // Distance to the closest boundary is returned in safety argument.
  //
  // Returns true if transformation matrix of the dynos gets changed.
  //
  // In future, when an inter-tringula boundary is crossed this will
  // also pass the dyno to neighbouring tringula.

  Bool_t trans_changed = false;

  Int_t np = mParaSurf->n_edge_planes();

  Float_t min_d = 1e5;
  for (Int_t p = 0; p < np; ++p)
  {
    Opcode::Point& pos     = * (Opcode::Point*) dyno->ref_trans().ArrT();
    Opcode::Point& old_pos = * (Opcode::Point*) dyno->ref_last_trans().ArrT();
    Float_t d = mParaSurf->edge_planes()[p].Distance(pos);
    if (d > 0)
    {
      trans_changed = true;
      handle_edge_crossing(*dyno, old_pos, pos, p, d);
      d = mParaSurf->edge_planes()[p].Distance(pos);
    }
    d = -d;
    if (d < min_d)
      min_d = d;
  }

  safety = min_d;

  return trans_changed;
}

//==============================================================================

void Tringula::DoFullBoxPrunning(Bool_t accumulate, Bool_t verbose)
{
  // Du full box-prunning step:
  // - fill prunning list with statos, dynos and flyers,
  // - perform box prunning,
  // - do detail checks on collision candidates.
  //
  // Output: mItsLines contain intersection segments from all collisions.
  //
  // Mainly here to compare speed against split-box-prunning.

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

  printf("%son %3u: res=%d, npairs=%3u, time=%f\n", _eh.Data(),
         nboxes, res, pairs.GetNbPairs(), time.TimeUntilNow().ToDouble());

  if (accumulate)
  {
    mItsLines.reserve(mItsLines.size() + 2*pairs.GetNbPairs());
  }
  else
  {
    mItsLines.clear();
    mItsLines.reserve(2*pairs.GetNbPairs());
  }

  const Text_t* debug_prefix = verbose ? "        " : 0;

  AABBTreeCollider collider;

  for (UInt_t i = 0; i < pairs.GetNbPairs(); ++i)
  {
    const Pair& p  = * pairs.GetPair(i);
    if (verbose)
      printf("  %3u:", i);

    Extendio::intersect_extendios(extarr[p.id0], extarr[p.id1], collider,
                                  mItsLines, debug_prefix);
  }
  printf(" Vector size = %zu, segments per pair = %f\n",
         mItsLines.size(), (float)mItsLines.size()/pairs.GetNbPairs());
}

void Tringula::DoSplitBoxPrunning()
{
  // Extendio-extendio collision detection and handling routine.

  static const Exc_t _eh("Tringula::DoSplitBoxPrunning ");

  // GTime  time(GTime::I_Now);

  setup_box_pruner();

  Opcode::Pairs ds_pairs;
  mBoxPruner->BipartitePruning(ds_pairs, 0, 1);

  Opcode::Pairs dd_pairs;
  mBoxPruner->CompletePruning(dd_pairs, 1);

  /*
    printf("Box-o-pruno on %d/%d: pairs=%d (%d/%d), time=%f\n",
    mBoxPruner->ListSize(0), mBoxPruner->ListSize(1),
    ds_pairs.GetNbPairs() + dd_pairs.GetNbPairs(),
    ds_pairs.GetNbPairs(),  dd_pairs.GetNbPairs(),
    time.TimeUntilNow().ToDouble());
  */

  // For mesh-mesh collisions.
  Opcode::AABBTreeCollider    collider;
  Extendio::CollisionSegments segments;

  // For COM inside tests.
  Opcode::RayCollider      RC;
  Opcode::CollisionFaces   CF;
  RC.SetDestination(&CF);
  RC.SetCulling(false);

  for (UInt_t i = 0; i < ds_pairs.GetNbPairs(); ++i)
  {
    using namespace Opcode;

    const Pair *p  = ds_pairs.GetPair(i);
    Statico   *stato = (Statico*)  mBoxPruner->GetUserData(0, p->id0);
    Dynamico  *dyno  = (Dynamico*) mBoxPruner->GetUserData(1, p->id1);
    // printf("    %2d %s, %s\n", i+1, stato->Identify().Data(), dyno->Identify().Data());

    Int_t ns = Extendio::intersect_extendios(stato, dyno, collider, segments);
    if (ns > 0)
    {
      // Calculate centers of mass for dyno and stato in global coordinates.

      ++stato->mNDynoColls;

      HTransF& t_dyno = dyno->ref_trans();
      Point com_dyno;
      t_dyno.MultiplyVec3(dyno->GetMesh()->RefCOM(), 1.0f, com_dyno);

      HTransF& t_stato = stato->ref_trans();
      Point com_stato;
      t_stato.MultiplyVec3(stato->GetMesh()->RefCOM(), 1.0f, com_stato);

      // Construct ray outwards from com_dyno in dir of com_dyno - com_stato.
      Opcode::Ray R;
      R.mOrig = com_dyno;
      R.mDir  = com_dyno - com_stato;

      Bool_t ok_p = Dynamico::handle_collision(dyno, stato, RC, R, com_dyno,
                                               segments);

      if (!ok_p)
        printf("%sDyno-Stato dyno->handle_collision failed.\n", _eh.Data());

      segments.Clear();
    }
  }

  for (UInt_t i = 0; i < dd_pairs.GetNbPairs(); ++i)
  {
    using namespace Opcode;

    const Pair *p  = dd_pairs.GetPair(i);
    Dynamico  *dyno0 = (Dynamico*) mBoxPruner->GetUserData(1, p->id0);
    Dynamico  *dyno1 = (Dynamico*) mBoxPruner->GetUserData(1, p->id1);
    // printf("    %2d %s, %s\n", i+1, dyno0->Identify().Data(), dyno1->Identify().Data());

    Int_t ns = Extendio::intersect_extendios(dyno0, dyno1, collider, segments);
    if (ns > 0)
    {
      Point up_dir;
      mParaSurf->pos2hdir(segments.RefCenter(), up_dir);

      Dynamico::handle_collision(dyno0, dyno1, up_dir, segments);

      segments.Clear();
    }

  }

  //printf("Box-o-pruno on %3u: res=%d, npairs=%3u, time=%f\n",
  //       nboxes, res, pairs.GetNbPairs(), time.TimeUntilNow().ToDouble());
}

/**************************************************************************/

void Tringula::TimeTick(Double_t t, Double_t dt)
{
  static const Exc_t _eh("Tringula::TimeTick ");

  GLensWriteHolder wlck(this);

  // Make time step for dynos, flyers
  {
    Stepper<> dyno_stepper(*mDynos);
    while (dyno_stepper.step())
    {
      Dynamico& D = * (Dynamico*) *dyno_stepper;
      D.TimeTick(t, dt);
    }

    Stepper<> flyo_stepper(*mFlyers);
    while (flyo_stepper.step())
    {
      Dynamico& D = * (Dynamico*) *flyo_stepper;
      D.TimeTick(t, dt);
    }

    Stepper<> tube_stepper(*mTubes);
    while (tube_stepper.step())
    {
      WSTube& D = * (WSTube*) *tube_stepper;
      D.TimeTick(t, dt);
    }
  }

  // Box-pruning, minimalistic collision handling.
  DoSplitBoxPrunning();


  // Update velocities, render-pos.
  {
    Stepper<> dyno_stepper(*mDynos);
    while (dyno_stepper.step())
    {
      Dynamico& D = * (Dynamico*) *dyno_stepper;

      D.update_aabb();
      D.update_last_data();
    }

    Stepper<> flyo_stepper(*mFlyers);
    while (flyo_stepper.step())
    {
      Dynamico& D = * (Dynamico*) *flyo_stepper;

      D.update_aabb();
      D.update_last_data();
    }
  }

  // Loop over TimeMakerClient children
  {
    Stepper<TimeMakerClient> tcl(this);
    while (tcl.step())
      tcl->TimeTick(t, dt);
  }
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
      D.mVVec.Zero();
      D.mWVec.Zero();
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
      t.OrtoNorm3Column(3, 1);
      t.SetBaseVecViaCross(2);
      break;
    }

  } // end switch
}

/**************************************************************************/

Bool_t Tringula::terrain_height(const Opcode::Point& pos, Float_t& point_h, Float_t& terrain_h)
{
  // Calculates height of given point and of the terrain at given pos.
  // Return true if all went ok.

  static const Exc_t _eh("Tringula::terrain_height ");

  Opcode::RayCollider    RC;
  RC.SetClosestHit(true);     // to keep the closes hit only
  Opcode::CollisionFaces CF;
  RC.SetDestination(&CF);

  Opcode::Point fgh;
  Opcode::Point hzero_pos;

  mParaSurf->pos2fgh(pos, fgh);
  point_h = fgh.z;
  fgh.z   = 0;
  mParaSurf->fgh2pos(fgh, hzero_pos);

  Opcode::Ray R;
  Float_t ray_offset = mParaSurf->pos2hray(hzero_pos, R);

  Bool_t cs = RC.Collide(R, *mMesh->GetOPCModel());
  if (cs && CF.GetNbFaces() == 1)
  {
    const Opcode::CollisionFace& cf = CF.GetFaces()[0];
    terrain_h = ray_offset - cf.mDistance;
    return true;
  }
  else
  {
    ISwarn(_eh + RC.CollideInfo(cs, R));
    return false;
  }

}

Bool_t Tringula::place_on_terrain(Statico* S, TriMesh* M, Bool_t check_inside,
                                  Float_t min_h_above)
{
  // Place statico on terrain so that the bounding box of the mesh
  // touches or penetrates the terrain. The distance is sampled on a
  // 3x3 grid.
  //
  // It is assumed that the statico is oriented along the local up
  // direction with zero height. If the trans matrix were fixed here,
  // the rotation would be somewhat poorly defined.
  //
  // check_inside - if true, the first miss of the terrain in the ray
  // collision test results in a termination of the loop over sampling
  // points. false is returned and no error message is printed.
  //
  // min_h_above - how much of statico's bbox must stick out of
  // terrain on each sampling-point. If this is not the case, false is
  // returned. If min_h_above <= 0, the check is not performed.
  //
  // Otherwise true is returned.

  static const Exc_t _eh("Tringula::place_on_terrain ");

  if (M == 0)
  {
    M = S->GetMesh();
    if (M == 0)
      throw(_eh + "TriMesh not passed as argument nor available from Statico.\n");
  }

  HTransF&      trans = S->ref_trans();
  Opcode::AABB& aabb  = M->ref_mesh_bbox();

  Float_t ray_offset = mParaSurf->GetMaxH() - aabb.GetMin(Opcode::_Z) +
    mParaSurf->GetEpsilon() +
    0.1f*(mParaSurf->GetMaxH() - mParaSurf->GetMinH()); // !! For local curvature, 1/10 of delta_h

  Float_t max_dist = 0, min_dist = ray_offset;

  Opcode::Point fdir  (trans.ArrX());
  Opcode::Point gdir  (trans.ArrY());
  Opcode::Point center(trans.ArrT());
  center.TMac(trans.ArrZ(), ray_offset);

  Opcode::Ray R;
  // R.mOrig set in loop
  R.mDir.Set(trans.ArrZ()); R.mDir.Neg();

  Opcode::RayCollider RC;
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

      Bool_t coll_status = RC.Collide(R, *mMesh->GetOPCModel());

      if (coll_status && CF.GetNbFaces() == 1)
      {
        max_dist = TMath::Max(max_dist, CF.GetFaces()[0].mDistance);
        min_dist = TMath::Min(min_dist, CF.GetFaces()[0].mDistance);
      }
      else
      {
        if (check_inside && coll_status)
          return false;
        else
          printf("%s(Statico*) sample_id %2d,%2d; status=%s, nfaces=%d\n"
                 "  nbvt=%d, nprt=%d, ni=%d\n"
                 "  ray_orig = %6.2f, %6.2f, %6.2f; ray_dir = %6.2f, %6.2f, %6.2f\n",
                 _eh.Data(), sf, sg, coll_status ? "ok" : "failed", CF.GetNbFaces(),
                 RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections(),
                 R.mOrig.x, R.mOrig.y, R.mOrig.z, R.mDir.x, R.mDir.y, R.mDir.z);
      }
    }
  }

  if (aabb.GetZSize() - (max_dist - min_dist) < min_h_above)
    return false;

  // printf("ray_offset = %f, max_dist = %f; to_move = %f\n",
  //        ray_offset, max_dist, ray_offset - max_dist);

  trans.MoveLF(3, ray_offset - max_dist);

  return true;
}

Bool_t Tringula::place_on_terrain(Dynamico* D, Float_t h_above)
{
  static const Exc_t _eh("Tringula::place_on_terrain ");

  Opcode::RayCollider    RC;
  RC.SetClosestHit(true);     // to keep the closes hit only
  Opcode::CollisionFaces CF;
  RC.SetDestination(&CF);

  Opcode::Point& pos = D->ref_pos();
  Opcode::Ray R;
  Float_t ray_offset = mParaSurf->pos2hray(pos, R);

  Int_t cs = RC.Collide(R, *mMesh->GetOPCModel());
  if (cs && CF.GetNbFaces() == 1)
  {
      const Opcode::CollisionFace& cf = CF.GetFaces()[0];
      pos.TMac(R.mDir, cf.mDistance - ray_offset - h_above);

      Float_t* n = mMesh->GetTTvor()->TriangleNormal(cf.mFaceID);

      HTransF& trans = D->ref_trans();
      trans.SetBaseVec(3, n);
      trans.OrtoNorm3Column(1, 3);
      trans.SetBaseVecViaCross(2);
      return true;
  }
  else
  {
    printf("%s(Dynamico*) status=%s, nfaces=%d\n"
           "  nbvt=%d, nprt=%d, ni=%d\n"
           "  ray_orig = %6.2f, %6.2f, %6.2f; ray_dir = %6.2f, %6.2f, %6.2f\n",
           _eh.Data(), cs ? "ok" : "failed", CF.GetNbFaces(),
           RC.GetNbRayBVTests(), RC.GetNbRayPrimTests(), RC.GetNbIntersections(),
           R.mOrig.x, R.mOrig.y, R.mOrig.z, R.mDir.x, R.mDir.y, R.mDir.z);
    return false;
  }
}

/**************************************************************************/
