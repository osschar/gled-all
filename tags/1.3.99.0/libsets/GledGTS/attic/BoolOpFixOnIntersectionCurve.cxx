// 2010.09.05
// Attempt to fix boolean ops by using intersection edges.
// Never got it to work.

namespace
{
  void triangle_other_edges(const GtsTriangle* t, const GtsEdge* e0,
			    GtsEdge*& e1, GtsEdge*& e2)
  {
    if (t->e1 == e0) {
      e1 = t->e2; e2 = t->e3;
    } else {
      e1 = t->e1; e2 = (t->e2 == e0) ? t->e3 : t->e2;
    }
  }

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

  void print_neigbour_triangles_on_other_edges(const GtsTriangle* t, const GtsEdge* e0)
  {
    GtsEdge *e1, *e2;
    triangle_other_edges(t, e0, e1, e2);
    printf("   Other triangles for e1\n");
    print_triangles(e1->triangles, t);
    printf("   Other triangles for e2\n");
    print_triangles(e2->triangles, t);
  }

  struct less_coord
  {
    bool operator()(const GtsVertex* v1, const GtsVertex* v2)
    {
      if (v1->p.x == v2->p.x) {
	if (v1->p.y == v2->p.y) {
	  return v1->p.z < v2->p.z;
	} else {
	  return v1->p.y < v2->p.y;
	}
      } else {
	return v1->p.x < v2->p.x;
      }
    }
  };

  struct equal_coord
  {
    bool operator()(const GtsVertex* v1, const GtsVertex* v2)
    {
      return v1->p.x == v2->p.x && v1->p.y == v2->p.y && v1->p.z == v2->p.z;
    }
  };

  struct vertex_hash
  {
    size_t operator()(const GtsVertex* v) const
    {
      hash<size_t> h; return h((size_t) (v->p.x + v->p.y + v->p.z));
    }
  };

  struct duplicate_vertex_remover
  {
    typedef map<GtsVertex*, GtsVertex*> map_by_ptr_t;
    typedef map_by_ptr_t::iterator      map_by_ptr_i;

    // typedef map<GtsVertex*, int, less_coord> map_by_coords_t;
    // typedef map_by_coords_t::iterator        map_by_coords_i;

    typedef hash_map<GtsVertex*, int, vertex_hash, equal_coord> map_by_coords_t;
    typedef map_by_coords_t::iterator                           map_by_coords_i;

    map_by_ptr_t     vv_map;     // maps to 0, if unique
    map_by_coords_t  vcoord_map;

    bool m_remove;

    duplicate_vertex_remover(bool remove) : m_remove(remove) {}

    map_by_ptr_i check_vertex(GtsVertex* v)
    {
      map_by_ptr_i vvi = vv_map.find(v);
      if (vvi == vv_map.end())
      {
	map_by_coords_i i = vcoord_map.find(v);
	if (i == vcoord_map.end())
	{
	  vvi = vv_map.insert(make_pair(v, (GtsVertex*)0)).first;
	}
	else
	{
	  vvi = vv_map.insert(make_pair(v, i->first)).first;
	  printf("Deleting vertex %p, cnt = %d; replacing with %p.\n", v, vcoord_map[v], i->first);
	  if (m_remove)
	  {
	    gts_vertex_replace (v, i->first);
	    gts_object_destroy (GTS_OBJECT (v));

	    //g_slist_free(v->segments);
	    //v->segments = 0;
	    //gts_object_destroy(GTS_OBJECT(v));
	  }
	}
	++vcoord_map[v];
      }
      return vvi;
    }

    void check_edge_insert_vertices(GtsEdge *e)
    {
      // Remap vertices if needed.
      // New vertices are inserted into the maps.

      map_by_ptr_i vvi1 = check_vertex(e->segment.v1);
      if (vvi1->second != 0)
      {
	printf("CEIV Replacing vertex 1 %p with vertex %p in edge %p.\n", e->segment.v1, vvi1->second, e);
	if (m_remove)
	{
	  gts_vertex_replace (e->segment.v1, vvi1->second);
	  gts_object_destroy (GTS_OBJECT (e->segment.v1));

	  // e->segment.v1 = vvi1->second;
	  // vvi1->second->segments = g_slist_prepend(vvi1->second->segments, &e->segment);
	}
      }

      map_by_ptr_i vvi2 = check_vertex(e->segment.v2);
      if (vvi2->second != 0)
      {
	printf("CEIV Replacing vertex 2 %p with vertex %p in edge %p.\n", e->segment.v2, vvi2->second, e);
	if (m_remove)
	{
	  gts_vertex_replace (e->segment.v2, vvi2->second);
	  gts_object_destroy (GTS_OBJECT (e->segment.v2));

	  // e->segment.v2 = vvi2->second;
	  // vvi2->second->segments = g_slist_prepend(vvi2->second->segments, &e->segment);
	}
      }

      if (vvi1->second != 0 && vvi1->second == vvi2->second)
      {
	printf("CEIV Warning ... trivial edge!\n");
	printf("   Nt = %u\n", g_slist_length(e->triangles));
	GtsPoint *p, *o;
	p = &e->segment.v1->p; printf("   %p %f %f %f\n", &e->segment.v1, p->x, p->y, p->z);
	o = p;

	GtsTriangle *t1  = (GtsTriangle*) e->triangles->data;
	GtsVertex   *v31 = gts_triangle_vertex_opposite(t1, e);
	p = &v31->p; printf("   %p | %f %f %f | %g %g %g\n", v31, p->x, p->y, p->z, o->x - p->x, o->y - p->y, o->z - p->z);
	print_neigbour_triangles_on_other_edges(t1, e);

	GtsTriangle *t2  = (GtsTriangle*) e->triangles->next->data;
	GtsVertex   *v32 = gts_triangle_vertex_opposite(t2, e);
	p = &v32->p; printf("   %p | %f %f %f | %g %g %g\n", v32, p->x, p->y, p->z, o->x - p->x, o->y - p->y, o->z - p->z);
	print_neigbour_triangles_on_other_edges(t2, e);
      }
    }

    void check_edge(GtsEdge *e)
    {
      // Remap vertices if needed. Only existing vertices are used.

      map_by_ptr_i vvi1 = vv_map.find(e->segment.v1);
      if (vvi1 != vv_map.end() && vvi1->second != 0)
      {
	printf("CE Replacing vertex 1 %p with vertex %p in edge %p.\n", e->segment.v1, vvi1->second, e);
	if (m_remove)
	{
	  // e->segment.v1 = vvi1->second;
	  // vvi1->second->segments = g_slist_prepend(vvi1->second->segments, &e->segment);
	}
      }

      map_by_ptr_i vvi2 = vv_map.find(e->segment.v2);
      if (vvi2 != vv_map.end() && vvi2->second != 0)
      {
	printf("CE Replacing vertex 2 %p with vertex %p in edge %p.\n", e->segment.v2, vvi2->second, e);
	if (m_remove)
	{
	  // e->segment.v2 = vvi2->second;
	  // vvi2->second->segments = g_slist_prepend(vvi2->second->segments, &e->segment);
	}
      }

      if (e->segment.v1 == e->segment.v2)
      {
	printf("CE Warning: edge %p collapsed into trivial edge!\n", e);
	printf("   Nt = %u\n", g_slist_length(e->triangles));
	GtsPoint *p;
	p = &e->segment.v1->p; printf("   %f %f %f\n", p->x, p->y, p->z);
	GtsTriangle *t1 = (GtsTriangle*) e->triangles->data;
	GtsTriangle *t2 = (GtsTriangle*) e->triangles->next->data;
	GtsVertex *v31 = gts_triangle_vertex_opposite(t1, e);
	p = &v31->p; printf("   %f %f %f\n", p->x, p->y, p->z);
	GtsVertex *v32 = gts_triangle_vertex_opposite(t2, e);
	p = &v32->p; printf("   %f %f %f\n", p->x, p->y, p->z);
      }
    }

    static void surf_edge_checker(GtsEdge* e, duplicate_vertex_remover* dvr)
    {
      //dvr->check_edge(e);
      dvr->check_edge_insert_vertices(e);
    }

    void vertex_check_blak(GSList *l)
    {
      printf("\nChecking intersection edges ...\n");
      int i = 0;
      do
      {
	GtsEdge   *e  = (GtsEdge*) l->data;
	l = g_slist_next(l);

	GtsVertex *v1 = e->segment.v1;
	GtsVertex *v2 = e->segment.v2;
	double len = gts_point_distance(&v1->p, &v2->p);
	int    ntr = g_slist_length(e->triangles);

	// printf("%-2d %p L=%22.16g, T=%d; v1 = %p (%22.16g, %22.16g, %22.16g), v2 = %p (%22.16g, %22.16g, %22.16g)\n",
	printf("%-2d %p L=%22.16g, T=%d; v1 = %p (%22.19f, %22.19f, %22.19f), v2 = %p (%22.19f, %22.19f, %22.19f)\n",
	       ++i, e, len, ntr, v1, v1->p.x, v1->p.y, v1->p.z, v2, v2->p.x, v2->p.y, v2->p.z);

	print_triangles(e->triangles, 0);

	/*
	  if (len < 1e-16 && ntr == 4)
	  {
	  GtsVertex *mid = gts_vertex_new(gts_vertex_class(),
	  0.5*(v1->p.x + v2->p.x),
	  0.5*(v1->p.y + v2->p.y),
	  0.5*(v1->p.z + v2->p.z));

	  printf("  Collapse valid = %d, Creates fold = %d\n",
	  gts_edge_collapse_is_valid(e),
	  gts_edge_collapse_creates_fold(e, mid, 0));

	  gts_object_destroy (GTS_OBJECT (e));
	      
	  gts_vertex_replace (v1, mid);
	  gts_object_destroy (GTS_OBJECT (v1));

	  gts_vertex_replace (v2, mid);
	  gts_object_destroy (GTS_OBJECT (v2));

	  // destroy duplicate edges
	  GSList *i = mid->segments;
	  while (i) {
	  GtsEdge * e1 = (GtsEdge*) i->data;
	  GtsEdge * duplicate;
	  while ((duplicate = gts_edge_is_duplicate (e1))) {
	  gts_edge_replace (duplicate, GTS_EDGE (e1));
	  // HEAP_REMOVE_EDGE (heap, duplicate);
	  gts_object_destroy (GTS_OBJECT (duplicate));
	  }
	  i = i->next;
	  if (!e1->triangles) {
	  // e1 is the result of the collapse of one edge of a pair of identical
	  // faces (it should not happen unless duplicate triangles are present in
	  // the initial surface)
	  g_warning ("file %s: line %d (%s): probably duplicate triangle.",
	  __FILE__, __LINE__, G_GNUC_PRETTY_FUNCTION);
	  // HEAP_REMOVE_EDGE (heap, e1);
	  gts_object_destroy (GTS_OBJECT (e1));
	  if (i == NULL) // mid has been destroyed
	  mid = NULL;
	  }
	  }

	  }
	*/

	check_edge_insert_vertices(e);
      }
      while (l != 0);
    }
  };
