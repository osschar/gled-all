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

namespace GTS {
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

  bMeasureTime = false;
  mRunTime     = 0;
}

/**************************************************************************/

void GTSRetriangulator::Coarsen()
{
  static const Exc_t _eh("GTSRetriangulator::Coarsen ");

  using namespace GTS;

  GTSurf* target = *mTarget;
  if(target == 0) {
    throw(_eh + "Link Target should be set.");
  }
  if(target == 0) {
    throw(_eh + "Link Target should be set.");
  }
  GtsSurface* s = target->CopySurface();
  if(s == 0) {
    throw(_eh + "Target should have non-null surface.");
  }

  GtsStopFunc l_stop_func = 0;
  gpointer    l_stop_data = 0;
  switch(mStopOpts) {
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
  switch (mCostOpts) {
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
  switch (mMidvertOpts) {
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
  if(bMeasureTime) start_time = new ::GTime(::GTime::I_Now);

  gts_surface_coarsen(s,
		      l_cost_func, l_cost_data,
		      l_coarsen_func, l_coarsen_data,
		      l_stop_func, l_stop_data,
		      mMinAngleDeg*TMath::Pi()/180);

  if(bMeasureTime) SetRunTime(start_time->TimeUntilNow().ToDouble());

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
  if(target == 0) {
    throw(_eh + "Link Target should be set.");
  }
  if(target == 0) {
    throw(_eh + "Link Target should be set.");
  }
  GtsSurface* s = target->CopySurface();
  if(s == 0) {
    throw(_eh + "Target should have non-null surface.");
  }

  GtsStopFunc l_stop_func = 0;
  gpointer    l_stop_data = 0;
  switch(mStopOpts) {
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
  switch (mCostOpts) {
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
  if(bMeasureTime) start_time = new ::GTime(::GTime::I_Now);

  gts_surface_refine(s,
		     l_cost_func, l_cost_data,
		     0, 0,
		     l_stop_func, l_stop_data);

  if(bMeasureTime) SetRunTime(start_time->TimeUntilNow().ToDouble());

  target->WriteLock();
  target->ReplaceSurface(s);
  target->WriteUnlock();
}

/**************************************************************************/
