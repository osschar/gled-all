// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Extendio
//
//

#include "Extendio.h"
#include "Glasses/ZImage.h"
#include "Extendio.c7"

#include "Tringula.h"

ClassImp(Extendio);

/**************************************************************************/

void Extendio::_init()
{
  mTringula = 0;

  bExploding = false;

  mLastTransPtr = &mTrans;
  mLastAABBPtr  = &mAABB;

  mDebugBits = 0;
}

Extendio::Extendio(const Text_t* n, const Text_t* t) :
  ZGlass(n,t),
  mHitPoints(100, -100, 100)
{
  _init();
}

Extendio::~Extendio()
{}

//==============================================================================

void Extendio::SetTringula(Tringula* tring)
{
  // Set tringula to which the extendio is attached.
  // Sub-classes override this to reinitialize cached data.

  mTringula = tring;
}

//==============================================================================

void Extendio::TakeDamage(Float_t damage)
{
  Bool_t was_exploding = bExploding;
  mHitPoints.Delta(-damage);
  if (mHitPoints.Get() < 0 && !was_exploding)
  {
    bExploding = true;
    mTringula->ExtendioExploding(this);
  }
}

//==============================================================================
// Intersection utilities
//==============================================================================

Float_t Extendio::height_above_terrain(const Opcode::Point& down_dir)
{
  static const Exc_t _eh("Extendio::height_above_terrain ");

  Opcode::RayCollider    RC;
  RC.SetFirstContact(false);  // true to only take first hit (not closest!)
  RC.SetClosestHit(true);     // to keep the closes hit only
  Opcode::CollisionFaces CF;
  RC.SetDestination(&CF);

  Opcode::Ray R(ref_pos(), down_dir);

  bool cs = RC.Collide(R, *mTringula->GetMesh()->GetOPCModel());
  if (cs && CF.GetNbFaces() == 1)
  {
      const Opcode::CollisionFace& cf = CF.GetFaces()[0];
      return cf.mDistance;
  }
  else
  {
    ISwarn(_eh + RC.CollideInfo(cs, R));
    return 0;
  }

}

Int_t Extendio::collide_with_tringula(CollisionSegments& segments)
{
  static const Exc_t _eh("Extendio::collide_with_tringula ");

  Int_t count = 0;

  Opcode::AABBTreeCollider collider;
  Opcode::BVTCache         cache;
  cache.Model0 = mTringula->GetMesh()->GetOPCModel();
  cache.Model1 = get_opc_model();

  TringTvor *TT0 = mTringula->GetMesh()->GetTTvor();
  TringTvor *TT1 = GetMesh()->GetTTvor();
  HTransF    HT0;  // identity
  HTransF   &HT1 = ref_trans();

  Bool_t s0 = collider.Collide(cache, 0, HT1);
  Bool_t s1 = collider.GetContactStatus();
  UInt_t np = collider.GetNbPairs();

  if (!s0)
  {
    printf("%scrappy collision status - result=%d, contact=%d, n_pairs=%u!\n",
           _eh.Data(), s0, s1, np);
  }

  const Opcode::Pair *ps = collider.GetPairs();
  Opcode::Segment     segment;

  for (UInt_t j = 0; j < np; ++j)
  {
    if (intersect_triangle(TT0, TT1, HT0, HT1,
			   ps[j].id0, ps[j].id1,
			   segment))
    {
      segments.push_back(segment);
      ++count;
    }
  }

  return count;
}


//------------------------------------------------------------------------------
// Static intersection utilities
//------------------------------------------------------------------------------

bool Extendio::intersect_triangle(TringTvor* TT0, TringTvor* TT1,
                                  HTransF  & HT0, HTransF  & HT1,
                                  Int_t    tidx0, Int_t    tidx1,
                                  Opcode::Segment& segment,
                                  const Text_t* debug_prefix)
{
  // Calculate intersection line between triangle tidx0 from Extendio ext0
  // and triangle tidx1 from ext1 in parent coordinate system.
  // Output:
  //   segment - contains the endpoints of the intersection line
  //
  // Returns: false if triangles actually do not intersect.
  // !!!! This is not fully checked, only distance_pattern is.
  //
  // If debug_prefix is set, some information is dumped along the
  // path, each line prefixed with this string.

  static const Exc_t _eh("Extendio::intersect_triangle ");

  using namespace Opcode;

  Int_t* T0 = TT0->Triangle(tidx0);
  Int_t* T1 = TT1->Triangle(tidx1);

  // Transform triangle vertices to parent coordinate system.
  // [ If we ever get proper dynamics, transform also velocities.
  //   Or maybe transform the intersection lines or whatever. ]
  Point V0[3], V1[3];
  for (int k=0; k<3; ++k)
  {
    HT0.MultiplyVec3(TT0->Vertex(T0[k]), 1.0f, V0[k]);
    HT1.MultiplyVec3(TT1->Vertex(T1[k]), 1.0f, V1[k]);
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

  if (debug_prefix)
    printf("%sdistances = %f, %f, %f; sign_patern = %d\n", debug_prefix,
           dst[0], dst[1], dst[2], pat);

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
      return false;
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
    // uv[k].z = 0; // if this becomes relevant, now always used by x,y components.
  }
  Point duv;  duv.Sub(uv[1], uv[0]);

  if (debug_prefix)
    printf("%suv0 = % 5.3f, % 5.3f; uv1 = % 5.3f, % 5.3f; duv = % 5.3f, % 5.3f\n", debug_prefix,
           uv[0].x, uv[0].y, uv[1].x, uv[1].y, duv.x, duv.y);

  // Check if outside triangle limits
  Float_t t[3];    // intersection times holder
  Int_t   ti, ts;  // time index; selected (maximal) index

  // First point ... calculate intersection multipliers
  ti = ts = 0;
  if (uv[0].x < 0)
  {
    t[ti] = -uv[0].x / duv.x;
    if (t[ti] > 0.0f && t[ti] < 1.0f) ++ti;
  }
  if (uv[0].y < 0 )
  {
    t[ti] = -uv[0].y / duv.y;
    if (t[ti] > 0.0f && t[ti] < 1.0f) ++ti;
  }
  if (uv[0].x+uv[0].y > 1)
  {
    t[ti] = (1 - uv[0].x - uv[0].y) / (duv.x + duv.y);
    if (t[ti] > 0.0f && t[ti] < 1.0f) ++ti;
  }

  if (debug_prefix)
    printf("%sfirst pnt: ti=%d, t0=%f, t1=%f, ts=%d\n", debug_prefix,
           ti, t[0], t[1], ts);

  // First point ... fix if necessary; assume positive 't[ts]'!
  if (ti > 0)
  {
    if (ti >= 2)
    {
      if (ti == 3)
      {
        printf("%sWarning-1, all 3 conditions true! Ignoring last solution.\n", _eh.Data());
      }
      if (t[1] > t[0]) ts = 1;
    }
    ip[0].TMac2(e1, t[ts]*duv.x, e2, t[ts]*duv.y);

    if (debug_prefix)
      printf("%sfirst pnt: ti=%d, t0=%f, t1=%f, ts=%d\n", debug_prefix,
             ti, t[0], t[1], ts);
  }

  // Second point ... calculate intersection multipliers
  // Invert du, dv and at the end also sign of t[ts]
  ti = ts = 0;
  if (uv[1].x < 0)
  {
    t[ti] = uv[1].x / duv.x;
    if (t[ti] > 0.0f && t[ti] < 1.0f) ++ti;
  }
  if (uv[1].y < 0 )
  {
    t[ti] = uv[1].y / duv.y;
    if (t[ti] > 0.0f && t[ti] < 1.0f) ++ti;
  }
  if (uv[1].x+uv[1].y > 1)
  {
    t[ti] = - (1 - uv[1].x - uv[1].y) / (duv.x + duv.y);
    if (t[ti] > 0.0f && t[ti] < 1.0f) ++ti;
  }

  if (debug_prefix)
    printf("%ssecnd pnt: ti=%d, t0=%f, t1=%f, ts=%d\n", debug_prefix,
           ti, t[0], t[1], ts);

  // Second point ... fix if necessary; assume positive 't[ts]'!
  if (ti > 0)
  {
    if (ti >= 2)
    {
      if (ti == 3)
      {
        printf("%sWarning-2, all 3 conditions true! Ignoring last solution.\n", _eh.Data());
      }
      if (t[1] > t[0]) ts = 1;
    }
    ip[1].TMac2(e1, -t[ts]*duv.x, e2, -t[ts]*duv.y);

    if (debug_prefix)
      printf("%ssecnd pnt: ti=%d, t0=%f, t1=%f, ts=%d\n", debug_prefix,
             ti, t[0], t[1], ts);
  }

  if (debug_prefix)
      printf("%s(%f,%f,%f), (%f, %f,%f) -- %f\n", debug_prefix,
             ip[0].x, ip[0].y, ip[0].z,
             ip[1].x, ip[1].y, ip[1].z, (ip[1]-ip[0]).Magnitude());


  // Copy the result
  segment.mP0 = ip[0];
  segment.mP1 = ip[1];

  return true;
}

//------------------------------------------------------------------------------

int Extendio::intersect_extendios(Extendio* ext0, Extendio* ext1,
                                  Opcode::AABBTreeCollider& collider,
                                  vector<Opcode::Segment> & segments,
                                  const Text_t* debug_prefix)
{
  // Intersect extendios ext0 and ext1.
  //
  // Output vector 'segments' is filled with line-segments of
  // intersecting triangles.
  //
  // Returns number of added segments.

  using namespace Opcode;

  int count = 0;

  BVTCache cache;
  cache.Model0 = ext0->get_opc_model();
  cache.Model1 = ext1->get_opc_model();

  Bool_t s0 = collider.Collide(cache, ext0->ref_trans(), ext1->ref_trans());
  Bool_t s1 = collider.GetContactStatus();
  UInt_t np = collider.GetNbPairs();

  const Text_t* sec_debug_prefix = 0;
  TString foo;
  if (debug_prefix)
  {
    printf("%s .vs. %s; result=%d, contact=%d, n_pairs=%u\n",
           ext0->GetName(), ext1->GetName(), s0, s1, np);
    foo = debug_prefix;
    foo += "    ";
    sec_debug_prefix = foo.Data();
  }

  // Triangle pairs
  const Pair* ps = collider.GetPairs();
  Segment     segment;

  TringTvor* TT0 = ext0->GetMesh()->GetTTvor();
  TringTvor* TT1 = ext1->GetMesh()->GetTTvor();
  HTransF  & HT0 = ext0->ref_trans();
  HTransF  & HT1 = ext1->ref_trans();

  for (UInt_t j = 0; j < np; ++j)
  {
    if (debug_prefix)
      printf("%s%2u: tri_idx0=%3d tri_idx1=%3d\n", debug_prefix,
             j, ps[j].id0, ps[j].id1);

    if (Extendio::intersect_triangle(TT0, TT1, HT0, HT1,
                                     ps[j].id0, ps[j].id1,
                                     segment, sec_debug_prefix))
    {
      segments.push_back(segment);
      ++count;
    }
  }

  return count;
}


//==============================================================================
// Extendio::CollisionSegments
//==============================================================================

void Extendio::CollisionSegments::calculate_center()
{
  HPointD sum;
  for (iterator s = begin(); s != end(); ++s)
  {
    sum += s->mP0;
    sum += s->mP1;
  }
  sum *= 0.5 / size();
  mCenter.Set(sum.x, sum.y, sum.z);
}
