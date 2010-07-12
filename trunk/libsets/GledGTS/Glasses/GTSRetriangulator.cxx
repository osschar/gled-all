// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GTSRetriangulator
//
// Interface to GTS surface 'coarsen' and 'refine' functions.
// See GTS manual for details.
// Refine always uses the mid-vertex insertion function.

#include "GTSRetriangulator.h"
#include "GTSRetriangulator.c7"

#include <GTS/GTS.h>

#include <Gled/GTime.h>

#include <TMath.h>

#include <cmath>

/**************************************************************************/

namespace GTS
{
  gdouble cost_angle (GtsEdge * e)
  {
    if (e->triangles && e->triangles->next)
      return fabs (gts_triangles_angle ((GtsTriangle*)e->triangles->data,
					(GtsTriangle*)e->triangles->next->data));
    return G_MAXDOUBLE;
  }

  gboolean refine_stop_number (gdouble cost, guint number, guint * max)
  {
    if (number > *max)
      return TRUE;
    return FALSE;
  }

  gboolean refine_stop_cost (gdouble cost, guint number, gdouble * min)
  {
    if (cost < *min)
      return TRUE;
    return FALSE;
  }
}

/**************************************************************************/

ClassImp(GTSRetriangulator);

/**************************************************************************/

void GTSRetriangulator::_init()
{
  mTarget = 0;

  mStopOpts = SO_Number;
  mStopNumber = 1000;
  mStopCost   = 0.5;

  mCostOpts = CO_Length;
  mVO_VolumeWght   = 0.5;
  mVO_BoundaryWght = 0.5;
  mVO_ShapeWght    = 0;

  mMidvertOpts = MO_Midvert;

  mMinAngleDeg = 1;

  mOutOfCoreDelta = 1e-3;

  bMeasureTime = false;
  mRunTime     = 0;
}

/**************************************************************************/

void GTSRetriangulator::Coarsen()
{
  static const Exc_t _eh("GTSRetriangulator::Coarsen ");

  using namespace GTS;

  GTSurf* target = *mTarget;
  if (target == 0)
    throw _eh + "Link Target should be set.";
  GtsSurface* s = target->CopySurface();
  if (s == 0)
    throw _eh + "Target should have non-null surface.";

  GtsStopFunc l_stop_func = 0;
  gpointer    l_stop_data = 0;
  switch (mStopOpts)
  {
    case SO_Number:
      l_stop_func = (GtsStopFunc)gts_coarsen_stop_number;
      l_stop_data = &mStopNumber;
      break;
    case SO_Cost:
      l_stop_func = (GtsStopFunc)gts_coarsen_stop_cost;
      l_stop_data = &mStopCost;
      break;
    default:
      throw(_eh + "Unknown StopOpts.");
  }

  GtsVolumeOptimizedParams l_vo_params =
    { mVO_VolumeWght, mVO_BoundaryWght, mVO_ShapeWght };

  GtsKeyFunc  l_cost_func  = 0;
  gpointer    l_cost_data  = 0;
  switch (mCostOpts)
  {
    case CO_Length:
      break;
    case CO_Volume:
      l_cost_func = (GtsKeyFunc)gts_volume_optimized_cost;
      l_cost_data = &l_vo_params;
      break;
    case CO_Angle:
      l_cost_func = (GtsKeyFunc)cost_angle;
      break;
    default:
      throw(_eh + "Unknown CostOpts.");
  }

  GtsCoarsenFunc l_coarsen_func = 0;
  gpointer       l_coarsen_data = 0;
  switch (mMidvertOpts)
  {
    case MO_Midvert:
      break;
    case MO_Volume:
      l_coarsen_func = (GtsCoarsenFunc) gts_volume_optimized_vertex;
      l_coarsen_data = &l_vo_params;
      break;
    default:
      throw(_eh + "Unknown MidvertOpts.");
  }

  ::GTime* start_time = 0;
  if (bMeasureTime) start_time = new ::GTime(::GTime::I_Now);

  gts_surface_coarsen(s,
		      l_cost_func, l_cost_data,
		      l_coarsen_func, l_coarsen_data,
		      l_stop_func, l_stop_data,
		      mMinAngleDeg*TMath::Pi()/180);

  if (bMeasureTime) SetRunTime(start_time->TimeUntilNow().ToDouble());

  target->WriteLock();
  target->ReplaceSurface(s);
  target->WriteUnlock();
}

/**************************************************************************/

void GTSRetriangulator::Refine()
{
  static const Exc_t _eh("GTSRetriangulator::Refine ");

  using namespace GTS;

  GTSurf* target = *mTarget;
  if (target == 0)
    throw _eh + "Link Target should be set.";
  GtsSurface* s = target->CopySurface();
  if (s == 0)
    throw _eh + "Target should have non-null surface.";

  GtsStopFunc l_stop_func = 0;
  gpointer    l_stop_data = 0;
  switch(mStopOpts)
  {
    case SO_Number:
      l_stop_func = (GtsStopFunc)refine_stop_number;
      l_stop_data = &mStopNumber;
      break;
    case SO_Cost:
      l_stop_func = (GtsStopFunc)refine_stop_cost;
      l_stop_data = &mStopCost;
      break;
    default:
      throw(_eh + "Unknown StopOpts.");
  }

  GtsVolumeOptimizedParams l_vo_params =
    { mVO_VolumeWght, mVO_BoundaryWght, mVO_ShapeWght };

  GtsKeyFunc  l_cost_func  = 0;
  gpointer    l_cost_data  = 0;
  switch (mCostOpts)
  {
    case CO_Length:
      break;
    case CO_Volume:
      l_cost_func = (GtsKeyFunc)gts_volume_optimized_cost;
      l_cost_data = &l_vo_params;
      break;
    case CO_Angle:
      l_cost_func = (GtsKeyFunc)cost_angle;
      break;
    default:
      throw(_eh + "Unknown CostOpts.");
  }

  ::GTime* start_time = 0;
  if (bMeasureTime) start_time = new ::GTime(::GTime::I_Now);

  gts_surface_refine(s,
		     l_cost_func, l_cost_data,
		     0, 0,
		     l_stop_func, l_stop_data);

  if (bMeasureTime) SetRunTime(start_time->TimeUntilNow().ToDouble());

  target->WriteLock();
  target->ReplaceSurface(s);
  target->WriteUnlock();
}

//==============================================================================

namespace
{
  using namespace GTS;

  void bbox_vertex_foo(GtsVertex* v, GtsBBox* bbox)
  {
    GtsPoint &p = v->p;
    GtsBBox  &b = *bbox;
    if (p.x < b.x1) b.x1 = p.x;
    if (p.x > b.x2) b.x2 = p.x;
    if (p.y < b.y1) b.y1 = p.y;
    if (p.y > b.y2) b.y2 = p.y;
    if (p.z < b.z1) b.z1 = p.z;
    if (p.z > b.z2) b.z2 = p.z;
  }

  void cluster_face_adder_foo(GtsFace* f, GtsClusterGrid* c_grid)
  {
    GtsVertex* vp[3];
    gts_triangle_vertices(&f->triangle, &vp[0], &vp[1], &vp[2]);
    gts_cluster_grid_add_triangle(c_grid, &vp[0]->p, &vp[1]->p, &vp[2]->p, NULL);
  }
}

void GTSRetriangulator::OutOfCoreSimplification()
{
  static const Exc_t _eh("GTSRetriangulator::OutOfCoreSimplification ");

  using namespace GTS;

  GTSurf* target = *mTarget;
  if (target == 0)
    throw _eh + "Link Target should be set.";
  GtsSurface* s = target->get_surface();
  if (s == 0)
    throw _eh + "Target should have non-null surface.";

  ::GTime* start_time = 0;
  if (bMeasureTime) start_time = new ::GTime(::GTime::I_Now);

  GtsSurface *d = MakeDefaultSurface();

  const Double_t big = 1e100;
  GtsBBox *bbox = gts_bbox_new(gts_bbox_class(), d, 0, 0, 0, 0, 0, 0);
  bbox->x1 = bbox->y1 = bbox->z1 =  big;
  bbox->x2 = bbox->y2 = bbox->z2 = -big;
  gts_surface_foreach_vertex(s, (GtsFunc)bbox_vertex_foo, bbox);

  printf("Boundingbox is (%f,%f,%f)-(%f,%f,%f)\n", bbox->x1, bbox->y1, bbox->z1, bbox->x2, bbox->y2, bbox->z2);

  GtsClusterGrid *c_grid = gts_cluster_grid_new(gts_cluster_grid_class(),
						gts_cluster_class(), 
						d, bbox, mOutOfCoreDelta);
  gts_surface_foreach_face(s, (GTS::GtsFunc) cluster_face_adder_foo, c_grid);

  GtsRange c_stats = gts_cluster_grid_update(c_grid);

  gts_object_destroy(GTS_OBJECT(c_grid));
  gts_object_destroy(GTS_OBJECT(bbox));

  printf ("%d clusters of size: min: %g avg: %.1f|%.1f max: %g\n",
	   c_stats.n, c_stats.min, c_stats.mean, c_stats.stddev, c_stats.max);

  if (bMeasureTime) SetRunTime(start_time->TimeUntilNow().ToDouble());

  target->WriteLock();
  target->ReplaceSurface(d);
  target->WriteUnlock();
}
