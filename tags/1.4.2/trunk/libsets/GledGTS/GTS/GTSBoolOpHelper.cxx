// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTSBoolOpHelper.h"
#include "Glasses/GTSurf.h"

//==============================================================================
//==============================================================================

namespace
{

  using namespace GTS;

  //----------------------------------------------------------------------------
  // Typedefs, structs for stl
  //----------------------------------------------------------------------------

  typedef pair<GtsTriangle*, GtsTriangle*> pTriTri_t;

  struct tri_pair_hash
  {
    hash<size_t> hfoo;

    size_t operator()(const pTriTri_t& a) const
    {
      return hfoo((size_t) a.first + (size_t) a.second);
    }
  };

  struct tri_pair_any_equal
  {
    bool operator()(const pTriTri_t& a, const pTriTri_t& b)
    {
      return (a.first == b.first  && a.second == b.second) ||
	     (a.first == b.second && a.second == b.first);
    }
  };

  typedef hash_set<pTriTri_t, tri_pair_hash, tri_pair_any_equal> sppTriTri_t;
  typedef sppTriTri_t::iterator                                  sppTriTri_i;

  typedef set<GtsTriangle*>   spTri_t;
  typedef spTri_t::iterator   spTri_i;

  typedef set<GtsEdge*>       spEdge_t;
  typedef spEdge_t::iterator  spEdge_i;

  //----------------------------------------------------------------------------
  // General utilities
  //----------------------------------------------------------------------------

  double edge_len(GtsEdge* e)
  {
    return gts_point_distance(&e->segment.v1->p, &e->segment.v2->p);
  }

  GtsEdge* longest_edge(GtsTriangle *t)
  {
    double l1 = edge_len(t->e1);
    double l2 = edge_len(t->e2);
    double l3 = edge_len(t->e3);

    if (l1 >= l2 && l1 >= l3) return t->e1;
    return (l2 >= l3) ? t->e2 : t->e3;
  }

  GtsEdge* shortest_edge(GtsTriangle *t)
  {
    double l1 = edge_len(t->e1);
    double l2 = edge_len(t->e2);
    double l3 = edge_len(t->e3);

    if (l1 <= l2 && l1 <= l3) return t->e1;
    return (l2 <= l3) ? t->e2 : t->e3;
  }

  GtsTriangle* edge_other_triangle(GtsEdge* e, GtsTriangle* t)
  {
    assert(g_slist_length(e->triangles) == 2);
    return (GtsTriangle*) (e->triangles->data == t ? e->triangles->next->data : e->triangles->data);
  }

  void triangle_other_edges(const GtsTriangle* t, const GtsEdge* e0,
			    GtsEdge*& e1, GtsEdge*& e2)
  {
    if (t->e1 == e0) {
      e1 = t->e2; e2 = t->e3;
    } else {
      e1 = t->e1; e2 = (t->e2 == e0) ? t->e3 : t->e2;
    }
  }

  void remove_duplicate_edges(GSList* edges)
  {
    while (edges)
    {
      GtsEdge * e1 = (GtsEdge*) edges->data;
      GtsEdge * duplicate;
      while ((duplicate = gts_edge_is_duplicate(e1)))
      {
	gts_edge_replace(duplicate, GTS_EDGE(e1));
	gts_object_destroy(GTS_OBJECT(duplicate));
      }
      edges = edges->next;
    }
  }

  void edge_collapse(GtsEdge* e, bool print_p=false)
  {
    GtsVertex *v1  = e->segment.v1;
    GtsVertex *v2  = e->segment.v2;
    GtsVertex *mid = gts_vertex_new(gts_vertex_class(),
				    0.5*(v1->p.x + v2->p.x),
				    0.5*(v1->p.y + v2->p.y),
				    0.5*(v1->p.z + v2->p.z));

    if (print_p)
      printf("  Collapse valid = %d, Creates fold = %d\n",
	     gts_edge_collapse_is_valid(e),
	     gts_edge_collapse_creates_fold(e, mid, 0.9));

    gts_object_destroy(GTS_OBJECT(e));

    gts_vertex_replace(v1, mid);
    gts_vertex_replace(v2, mid);

    gts_object_destroy(GTS_OBJECT(v1));
    gts_object_destroy(GTS_OBJECT(v2));

    remove_duplicate_edges(mid->segments);
  }


  //----------------------------------------------------------------------------
  // Print / debug functions
  //----------------------------------------------------------------------------

  void print_triangles(GSList* trings, const GtsTriangle* nt)
  {
    int i = 0;
    while (trings)
    {
      GtsTriangle *t = (GtsTriangle*) trings->data;

      printf("      %d. a=%g, p=%g, q=%g", ++i, gts_triangle_area(t), gts_triangle_perimeter(t), gts_triangle_quality(t));

      if (t == nt)
	printf(" *** ");
      printf("\n");
      
      trings = trings->next;
    }
  }

  struct face_print_arg
  {
    double area_limit;
    int    count;

    face_print_arg(double al) : area_limit(al), count(0) {}
  };

  void face_print(GtsFace* f, face_print_arg* a)
  {
    ++(a->count);
    GtsTriangle *t = &f->triangle;
    if (gts_triangle_area(t) < a->area_limit)
      printf("%3d. a=%22.18g, p=%22.18g, q=%22.18g\n", a->count,
	     gts_triangle_area(t), gts_triangle_perimeter(t), gts_triangle_quality(t));
  }

  void print_triangles_smaller_than_area(GtsSurface* s, double area_limit=1e-10)
  {
    face_print_arg arg(area_limit);
    gts_surface_foreach_face(s, (GTS::GtsFunc) face_print, &arg);
  }

  void print_neigbour_triangles_on_other_edges(const GtsTriangle* t, const GtsEdge* e0)
  {
    GtsEdge *e1, *e2;
    triangle_other_edges(t, e0, e1, e2);
    printf("   Other triangles for e1\n");
    print_triangles(e1->triangles, t);
    printf("   Other triangles for e2\n");
    print_triangles(e2->triangles, t);
  }

}

namespace GTS
{
  struct BoolOpHelperImpl
  {
    sppTriTri_t  tripair_set;
    spTri_t      tri_set;
    spEdge_t     edge_set;
  };
}

//==============================================================================
//==============================================================================

using namespace GTS;

BoolOpHelper::BoolOpHelper(GTSurf* tgt, GTSurf* a, GTSurf* b, const Exc_t& _eh) :
  target(tgt), a_surf(0), b_surf(0), inter(0), result(0),
  eps_a(0), eps_p(0), eps_l(0),
  debug(1),
  impl(new BoolOpHelperImpl)
{
  if (a == b) throw _eh + "Same value of argument a and b.";

  if (a)
  {
    a_surf = a->CopySurface();
    if (a_surf == 0) throw _eh + "Argument a has null surface.";

    if (a != target)
    {
      auto_ptr<ZTrans> from_a(ZNode::BtoA(target, a));
      if (*from_a == 0) throw _eh + "No common parent with a.";
      GTS::TransformSurfaceVertices(a_surf, from_a.get());
    }
  }

  if (b)
  {
    b_surf = b->CopySurface();
    if (b_surf == 0) throw _eh + "Argument b has null surface.";

    if (b != target)
    {
      auto_ptr<ZTrans> from_b(ZNode::BtoA(target, b));
      if (*from_b == 0) throw _eh + "No common parent with b.";
      GTS::TransformSurfaceVertices(b_surf, from_b.get());
    }
  }
}

BoolOpHelper::~BoolOpHelper()
{
  if (a_surf) gts_object_destroy(GTS_OBJECT(a_surf));
  if (b_surf) gts_object_destroy(GTS_OBJECT(b_surf));
  if (inter)  gts_object_destroy(GTS_OBJECT(inter));
  if (result) gts_object_destroy(GTS_OBJECT(result));
}

//------------------------------------------------------------------------------

void BoolOpHelper::BuildInter(const Exc_t& _eh)
{
  if (a_surf == 0) throw _eh + "Argument a is null.";
  if (b_surf == 0) throw _eh + "Argument b is null.";

  switch (target->GetPostBoolOp())
  {
    case GTSurf::PBM_Noop:
    {
      break;
    }
    case GTSurf::PBM_AsValues:
    {
      eps_a = target->GetPostBoolArea();
      eps_p = target->GetPostBoolPerimeter();
      eps_l = target->GetPostBoolLength();
      break;
    }
    case GTSurf::PBM_AsFractions:
    {
      GtsSurfaceQualityStats a_stats, b_stats;
      gts_surface_quality_stats(a_surf, &a_stats);
      gts_surface_quality_stats(b_surf, &b_stats);
      Double_t ma = TMath::Min(a_stats.face_area.min, b_stats.face_area.min);
      Double_t ml = TMath::Min(a_stats.edge_length.min, b_stats.edge_length.min);
      Double_t mp = 3.0 * ml;
      eps_a = ma * target->GetPostBoolArea();
      eps_p = mp * target->GetPostBoolPerimeter();
      eps_l = ml * target->GetPostBoolLength();
      break;
    }
  }

  GNode *a_tree    = gts_bb_tree_surface(a_surf);
  bool   a_is_open = gts_surface_volume(a_surf) < 0;

  GNode *b_tree    = gts_bb_tree_surface(b_surf);
  bool   b_is_open = gts_surface_volume(b_surf) < 0;

  inter = gts_surface_inter_new(gts_surface_inter_class(),
				a_surf, b_surf, a_tree, b_tree,
				a_is_open, b_is_open);
  gts_bb_tree_destroy(a_tree, true);
  gts_bb_tree_destroy(b_tree, true);

  gboolean orientable, closed;
  orientable = gts_surface_inter_check(inter, &closed);
  if (!orientable) throw _eh + "Intersection curve not orientable.";
  if (!closed)     throw _eh + "Intersection curve not closed.";
}

//------------------------------------------------------------------------------

void BoolOpHelper::PostProcess()
{
  if (eps_a == 0 || eps_p == 0)
  {
    if (debug > 0)
      printf("BoolOpHelper::PostProcess Limits zero ... nothing to be done.\n");
    return;
  }
  
  if (debug > 0)
    printf("BoolOpHelper::PostProcess Entering.\n");

  if (a_surf) { gts_object_destroy(GTS_OBJECT(a_surf)); a_surf = 0; }
  if (b_surf) { gts_object_destroy(GTS_OBJECT(b_surf)); b_surf = 0; }
  if (inter)  { gts_object_destroy(GTS_OBJECT(inter));  inter  = 0; }

  collapse_adjacent_epsi_triangles();

  handle_zeta_triangles();

  for (spEdge_i i = impl->edge_set.begin(); i != impl->edge_set.end(); ++i)
  {
    if (edge_len(*i) < eps_l)
      edge_collapse(*i);
  }

  if (debug > 0)
      printf("BoolOpHelper::PostProcess Done.\n");
}

//------------------------------------------------------------------------------

void BoolOpHelper::MakeMerge()
{
  result = GTS::MakeDefaultSurface();
  gts_surface_merge(result, a_surf);
  gts_surface_merge(result, b_surf);
}

void BoolOpHelper::MakeUnion()
{
  result = GTS::MakeDefaultSurface();

  gts_surface_inter_boolean(inter, result, GTS_1_OUT_2);
  gts_surface_inter_boolean(inter, result, GTS_2_OUT_1);

  if (target->GetPostBoolOp() != GTSurf::PBM_Noop)
    PostProcess();
}

void BoolOpHelper::MakeIntersection()
{
  result = GTS::MakeDefaultSurface();

  gts_surface_inter_boolean(inter, result, GTS_1_IN_2);
  gts_surface_inter_boolean(inter, result, GTS_2_IN_1);

  if (target->GetPostBoolOp() != GTSurf::PBM_Noop)
    PostProcess();
}

void BoolOpHelper::MakeDifference()
{
  result = GTS::MakeDefaultSurface();

  gts_surface_inter_boolean(inter, result, GTS_1_OUT_2);
  gts_surface_inter_boolean(inter, result, GTS_2_IN_1);
  gts_surface_foreach_face (inter->s2, (GtsFunc) gts_triangle_revert, 0);
  gts_surface_foreach_face (b_surf,    (GtsFunc) gts_triangle_revert, 0);

  if (target->GetPostBoolOp() != GTSurf::PBM_Noop)
    PostProcess();
}

//------------------------------------------------------------------------------

GtsSurface* BoolOpHelper::TakeResult()
{
  GtsSurface *ret = result;
  result = 0;
  return ret;
}


//==============================================================================
// Internal functions
//==============================================================================

//----------------------------------------------------------------------------
// Removal of a two adjacent triangles, both with area ~ 0, perimeter ~ 0.
// Collapse all edges into a single vertex.
//----------------------------------------------------------------------------

bool BoolOpHelper::is_epsi(GtsTriangle *t)
{
  return gts_triangle_area(t) < eps_a && gts_triangle_perimeter(t) < eps_p;
}

void BoolOpHelper::epsi_select(GtsFace* f, BoolOpHelper* boh)
{
  GtsTriangle *t = &f->triangle;
  if (boh->is_epsi(t))
  {
    boh->impl->tri_set.insert(t);
  }
}

void BoolOpHelper::epsi_pair_select(GtsFace* f, BoolOpHelper* boh)
{
  GtsTriangle *t = &f->triangle;
  if (boh->is_epsi(t))
  {
    GtsTriangle *ot;
    if ((ot = edge_other_triangle(t->e1, t), boh->is_epsi(ot)) ||
	(ot = edge_other_triangle(t->e2, t), boh->is_epsi(ot)) ||
	(ot = edge_other_triangle(t->e3, t), boh->is_epsi(ot)))
    {
      boh->impl->tripair_set.insert(make_pair(t, ot));
    }
  }
}

void BoolOpHelper::collapse_adjacent_epsi_triangles()
{
  sppTriTri_t& tripairs = impl->tripair_set;

  gts_surface_foreach_face(result, (GTS::GtsFunc) epsi_pair_select, this);

  if (debug > 0)
  {
    gts_surface_foreach_face(result, (GTS::GtsFunc) epsi_select, this);
    printf("Begin epsi pair removal, Nepsi_pair=%zu, Nepsi=%zu\n", tripairs.size(), impl->tri_set.size());
    impl->tri_set.clear();
  }

  while ( ! tripairs.empty())
  {
    GtsTriangle *t1 = tripairs.begin()->first, *t2 = tripairs.begin()->second;

    GtsEdge *common_edge = gts_triangles_common_edge(t1, t2);
    GtsVertex *v1 = common_edge->segment.v1, *v2 = common_edge->segment.v2;
    GtsVertex *mid = gts_vertex_new(gts_vertex_class(),
				    0.5*(v1->p.x + v2->p.x),
				    0.5*(v1->p.y + v2->p.y),
				    0.5*(v1->p.z + v2->p.z));

    GtsVertex *vts[4] = { v1, v2,
			  gts_triangle_vertex_opposite(t1, common_edge),
			  gts_triangle_vertex_opposite(t2, common_edge) };

    GtsEdge *egs[5] = { common_edge,
			gts_triangle_edge_opposite(t1, v1),
			gts_triangle_edge_opposite(t1, v2),
			gts_triangle_edge_opposite(t2, v1),
			gts_triangle_edge_opposite(t2, v2) };

    gts_allow_floating_vertices = gts_allow_floating_edges = true;

    for (int i = 0; i < 5; ++i)
    {
      gts_object_destroy(GTS_OBJECT(egs[i]));
    }
    for (int i = 0; i < 4; ++i)
    {
      gts_vertex_replace(vts[i], mid);
      gts_object_destroy(GTS_OBJECT(vts[i]));
    }

    gts_allow_floating_vertices = gts_allow_floating_edges = false;

    remove_duplicate_edges(mid->segments);

    tripairs.erase(tripairs.begin());
  }

  if (debug > 0)
  {
    gts_surface_foreach_face(result, (GTS::GtsFunc) epsi_pair_select, this);
    gts_surface_foreach_face(result, (GTS::GtsFunc) epsi_select, this);
    printf("End epsi pair removal, Nepsi_pair=%zu, Nepsi=%zu\n", tripairs.size(), impl->tri_set.size());
    tripairs.clear();
    impl->tri_set.clear();
  }
}


//----------------------------------------------------------------------------
// Handling of triangles with area ~ 0, perimeter > eps.
// First check shortest edge -> if length < eps, collapse shortest edge.
// Otherwise replace longest edge so that it connects the vertices
// opposite to the longest edge.
//----------------------------------------------------------------------------

bool BoolOpHelper::is_zeta(GtsTriangle *t)
{
  return gts_triangle_area(t) < eps_a && gts_triangle_perimeter(t) >= eps_p;
}

void BoolOpHelper::zeta_select(GtsFace* f, BoolOpHelper* boh)
{
  GtsTriangle *t = &f->triangle;
  if (boh->is_zeta(t))
  {
    boh->impl->tri_set.insert(t);
  }
}

void BoolOpHelper::handle_zeta_triangles()
{
  int n_collapsed = 0, n_reconnected = 0;

  spTri_t &tset = impl->tri_set;

  gts_surface_foreach_face(result, (GTS::GtsFunc) zeta_select, this);

  if (debug > 0)
  {
    printf("Begin zeta handling, Nzeta=%zu\n", tset.size());
  }

  while ( ! tset.empty())
  {
    GtsTriangle *t = * tset.begin();
    GtsEdge *el = longest_edge(t);
    GtsEdge *es = shortest_edge(t);

    if (debug > 1)
    {
      printf("a=%22.18g, p=%22.18g, q=%22.18g\n", gts_triangle_area(t), gts_triangle_perimeter(t), gts_triangle_quality(t));
      printf("  longest=%.18g (%d), shortest=%.18g (%d)\n",
	     edge_len(el), gts_edge_collapse_is_valid(el),
	     edge_len(es), gts_edge_collapse_is_valid(es));
      print_triangles(el->triangles, t);
      print_triangles(es->triangles, t);
    }

    GtsTriangle *to = 0;

    if (edge_len(es) > eps_p)
    {
      if (debug > 1)
      {
	printf(" Reconnecting vertices opposite to longest edge.\n");
      }

      to = edge_other_triangle(el, t);

      // This doesn't always work (was lucky to work on example with
      // intersection curve in y-z plane).
      // So ... get normal and do dot product with new triangle normals later.
      // bool orientation_is_positive = gts_triangle_orientation(to) > 0;
      double onx, ony, onz, nx, ny, nz;
      gts_triangle_normal(to, &onx, &ony, &onz);

      GtsVertex *v1 = gts_triangle_vertex_opposite(t,  el);
      GtsVertex *v2 = gts_triangle_vertex_opposite(to, el);

      GtsEdge *new_edge = gts_edge_new(gts_edge_class(), v1, v2);

      GtsFace *f1 = gts_face_new(gts_face_class(), new_edge,
				 gts_triangle_edge_opposite(t,  el->segment.v1),
				 gts_triangle_edge_opposite(to, el->segment.v1));
      gts_surface_add_face(result, f1);
      // if ((gts_triangle_orientation(&f1->triangle) > 0) != orientation_is_positive)
      gts_triangle_normal(&f1->triangle, &nx, &ny, &nz);
      if (nx*onx + ny*ony + nz*onz < 0)
	gts_triangle_revert(&f1->triangle);

      GtsFace *f2 = gts_face_new(gts_face_class(), new_edge,
				 gts_triangle_edge_opposite(t,  el->segment.v2),
				 gts_triangle_edge_opposite(to, el->segment.v2));
      gts_surface_add_face(result, f2);
      // if ((gts_triangle_orientation(&f2->triangle) > 0) != orientation_is_positive)
      gts_triangle_normal(&f2->triangle, &nx, &ny, &nz);
      if (nx*onx + ny*ony + nz*onz < 0)
	gts_triangle_revert(&f2->triangle);

      gts_object_destroy(GTS_OBJECT(el));

      // Collect shortest edges -- these can be safely
      // collapsed after the surface has been processed with this function.
      // Coarsening also removes them.
      // Edge-length limit is applied afterwards.
      impl->edge_set.insert(es);

      ++n_reconnected;
    }
    else
    {
      if (debug > 1)
      {
	printf(" Collapsing shortest edge.\n");
      }

      to = edge_other_triangle(es, t);

      edge_collapse(es, debug > 1);

      ++n_collapsed;
    }

    tset.erase(t);
    tset.erase(to);
  }

  if (debug > 0)
  {
    printf("End zeta handling, Ncollapsed=%d, Nreconnected=%d\n", n_collapsed, n_reconnected);
  }
}
