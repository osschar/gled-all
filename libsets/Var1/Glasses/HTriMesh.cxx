// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "HTriMesh.h"
#include "HTriMesh.c7"

// HTriMesh

//______________________________________________________________________________
//
// Hierarhical mesh
//
// Consists of:
// a) high-level hierarhical subdivision
//    all hierarchy represented by a single TringTvor;
//    nodes point into this tvor to identify triangles;
//    ?? depth first or breadth first ??
//    breadth first keeps triangles from a given level together
//    - good for opcode, to construct full tree at some level;
//      is this good? is it needed?
//    - also good for rendering, as some level will be selected at a given
//      distance (in space approach mode);
//    depth first keeps one brach together
//    - good for recursive queries / descends?
// b) low-level triangle soup attached to a leaf-node
//    for now, keep all of them in own TringTvor
//
// In principle could have several a-like layers.
// Or simplify the thing by always having a single a-layer -- but this then
// leads to duplication of vertices.
//
// Hmmh ... aren't vertices always shared?
// Yes ... but at some point I'd like to move leaf-vertices into separate
// meshes, managed by separate tringulas.
//
// So? At this intermediate level ... what do I do?
// Duplicate a-level vertices when creating b-mesh -- to keep Tringula ok.
// Then, it will also become important if one sees Flyers -- checking whole
// bounding volume of the triangle. Hmm, all Extendios ... but they don't go
// that hight so one could just hack highest point.
//
// 


ClassImp(HTriMesh);

//==============================================================================

void HTriMesh::_init()
{}

HTriMesh::HTriMesh(const Text_t* n, const Text_t* t) :
  TriMesh(n, t)
{
  _init();
}

HTriMesh::~HTriMesh()
{}

//==============================================================================

void HTriMesh::SetDrawLevel(Int_t l)
{
  Int_t l_max = mLevels.size() - 1;
  if (l < 0) l = 0;
  if (l > l_max) l = l_max;
  mDrawLevel = l;
  Stamp(FID());
}

void HTriMesh::PrintLevels()
{
  Int_t l_max = mLevels.size() - 1;
  for (Int_t i = 0; i <= l_max; ++i)
  {
    HLevel &l = mLevels[i];
    printf("%2d %2d %6d %6d %6d %zu\n", i, l.fLevel, l.fFirstT, l.fNT, l.LastT(), l.fNodes.size());
  }
}

//==============================================================================

void HTriMesh::subdivide_hierarhical(TringTvorSubdivider& tts)
{
  // Input level with parent nodes
  //       starting triangle index, or reference to it
  //       edge map
  // Assumes triangles and vertices are already allocated.
  //   Uses some function to do sub-division.
  //   Will I need to move them afterwards? ParaSurf knows ...

  HLevel &prev_level = mLevels[tts.fCurLevel];
  HLevel &new_level  = mLevels[tts.fCurLevel + 1];

  assert(prev_level.fLevel == tts.fCurLevel);
  assert(prev_level.fFirstT + prev_level.fNT == tts.fCurT);

  new_level.Init(tts.fCurLevel + 1, tts.fCurT, 4 * prev_level.fNT, prev_level.fNT);

  printf("HTriMesh::subdivide_hierarhical lvl=%d, t0_in=%d, nt_in=%d; current_t=%d, current_v=%d\n",
	 tts.fCurLevel, prev_level.fFirstT, prev_level.fNT, tts.fCurT, tts.fCurV);
  fflush(stdout);

  for (vpHNode_i ni = prev_level.fNodes.begin(); ni != prev_level.fNodes.end(); ++ni)
  {
    HNode &prev_node = **ni;
    // printf("  prv_level size=%zu, capacity=%zu\n", prev_node.fSubNodes.size(), prev_node.fSubNodes.capacity());
    for (Int_t t = prev_node.FirstT(); t <= prev_node.LastT(); ++t)
    {
      Int_t first_t = tts.SubdivideTriangle(t);
      // printf("    t=%4d, first_t=%d\n", t, first_t);
      fflush(stdout);
      prev_node.fSubNodes.push_back(new HNode(t, first_t, 4));
      new_level.fNodes.push_back(prev_node.fSubNodes.back());
    }
    // printf("  prv_level size=%zu, capacity=%zu\n", prev_node.fSubNodes.size(), prev_node.fSubNodes.capacity());
  }

  printf("  new_level size=%zu, capacity=%zu\n", new_level.fNodes.size(), new_level.fNodes.capacity());

  ++tts.fCurLevel;
}

void HTriMesh::subdivide_leaf(TringTvorSubdivider& tts, Int_t n_leaf)
{
  assert((size_t) tts.fCurLevel == mLevels.size() - 2);

  HLevel &prev_level = mLevels[tts.fCurLevel];
  HLevel &new_level  = mLevels[tts.fCurLevel + 1];

  new_level.Init(tts.fCurLevel + 1, tts.fCurT, (1 << 2*n_leaf) * prev_level.fNT, prev_level.fNT);

  printf("HTriMesh::subdivide_leaf lvl=%d, t0_in=%d, nt_in=%d; current_t=%d, current_v=%d\n",
	 tts.fCurLevel, prev_level.fFirstT, prev_level.fNT, tts.fCurT, tts.fCurV);
  fflush(stdout);

  for (vpHNode_i ni = prev_level.fNodes.begin(); ni != prev_level.fNodes.end(); ++ni)
  {
    HNode &prev_node = **ni;
    for (Int_t t = prev_node.FirstT(); t <= prev_node.LastT(); ++t)
    {
      Int_t *v = tts.fTvor.Triangle(t);
      Int_t first_t = tts.fCurT;
      subdivide_leaf_rec(tts, v[0], v[1], v[2], n_leaf - 1);

      // or sth like this ... recheck!!
      assert(tts.fCurT - first_t == 4 << (2 * (n_leaf - 1)));

      prev_node.fSubNodes.push_back(new HNode(t, first_t, tts.fCurT - first_t));
      new_level.fNodes.push_back(prev_node.fSubNodes.back());
    }

    // printf("  prv_level size=%zu, capacity=%zu\n", prev_node.fSubNodes.size(), prev_node.fSubNodes.capacity());
  }

  printf("  new_level size=%zu, capacity=%zu\n", new_level.fNodes.size(), new_level.fNodes.capacity());

  ++tts.fCurLevel;
}

void HTriMesh::subdivide_leaf_rec(TringTvorSubdivider& tts, Int_t v0, Int_t v1, Int_t v2, Int_t depth)
{
  Int_t u0 = tts.SubdivideEdge(v0, v1);
  Int_t u1 = tts.SubdivideEdge(v1, v2);
  Int_t u2 = tts.SubdivideEdge(v2, v0);

  if (depth > 0)
  {
    --depth;
    subdivide_leaf_rec(tts, v0, u0, u2, depth);
    subdivide_leaf_rec(tts, u0, v1, u1, depth);
    subdivide_leaf_rec(tts, u1, v2, u2, depth);
    subdivide_leaf_rec(tts, u2, u0, u1, depth);
  }
  else
  {
    tts.fTvor.SetTriangle(tts.fCurT++, v0, u0, u2);
    tts.fTvor.SetTriangle(tts.fCurT++, u0, v1, u1);
    tts.fTvor.SetTriangle(tts.fCurT++, u1, v2, u2);
    tts.fTvor.SetTriangle(tts.fCurT++, u2, u0, u1);
  }
}

//------------------------------------------------------------------------------

void HTriMesh::Subdivide(Int_t n_hierarhical, Int_t n_leaf)
{
  static const Exc_t _eh("HTriMesh::Subdivide ");

  assert_tvor(_eh);
  TringTvor &T = *mTTvor;

  T.DeleteSecondaryArrays();

  mRootNode.Init(-1, 0, T.NTrings());

  mLevels.resize(1 + n_hierarhical + (n_leaf > 0 ? 1 : 0));
  mLevels[0].Init(0, 0, T.NTrings(), T.NTrings());
  mLevels[0].fNodes.push_back(&mRootNode);

  TringTvorSubdivider tts(T);
  tts.BeginSubdivision(n_hierarhical, n_leaf);

  for (Int_t i = 0; i < n_hierarhical; ++i)
  {
    printf("HTriMesh::Subdivide hierarhical, i = %d\n", i);
    fflush(stdout);
    subdivide_hierarhical(tts);
  }

  if (n_leaf > 0)
  {
    printf("HTriMesh::Subdivide leaf, going in for n_leaf = %d\n", n_leaf);
    fflush(stdout);
    subdivide_leaf(tts, n_leaf);
  }

  tts.EndSubdivision();
}


//==============================================================================
// HTriMesh::TringTvorSubdivider
//==============================================================================

void HTriMesh::TringTvorSubdivider::BeginSubdivision(Int_t n_hierarhical, Int_t n_leaf)
{
  // Initialization function. Could be joined into ctor.

  fCurV = fTvor.NVerts();
  fCurT = fTvor.NTrings();
  fCurLevel = 0;

  // Count edges
  Int_t ne = TriMesh::fill_edge_map(fTvor.NTrings(), fTvor.Trings(), fEdgeMap, fTvor.mNVerts);
  fEdgeMap.clear();

  Int_t nnv = 0;
  Int_t nep = ne;
  Int_t nnt = 0;
  Int_t ntp = fCurT;
  for (Int_t i = 0; i < n_hierarhical; ++i)
  {
    nnv += nep;
    nep  = 2 * nep + 3 * ntp;
    ntp *= 4;
    nnt += ntp;
    printf("BS-h i=%d: nnv=%d nnt=%d;  nep=%d ntp=%d\n", i, nnv, nnt, nep, ntp);
  }
  Int_t nt_hierarhical = nnt;
  for (Int_t i = 0; i < n_leaf; ++i)
  {
    nnv += nep;
    nep  = 2 * nep + 3 * ntp;
    ntp *= 4;
    nnt += ntp;
    printf("BS-l i=%d: nnv=%d nnt=%d;  nep=%d ntp=%d\n", i, nnv, nnt, nep, ntp);
  }
  Int_t nt_leaf = ntp;

  fTvor.AddVertices (nnv);
  fTvor.AddTriangles(nt_hierarhical + nt_leaf);
}

Int_t HTriMesh::TringTvorSubdivider::SubdivideEdge(Int_t v0, Int_t v1)
{
  // Returns index of vertex between v0 and v1.
  // It is possible that the edge was already subdivided.
  // When a new vertex is created, it is inserted into fEdgeMap.

  HTriMesh::hEdge_i i = fEdgeMap.find(Edge(v0, v1));
  if (i != fEdgeMap.end())
    return i->second;

  HPointF &p0 = * (HPointF*) fTvor.Vertex(v0);
  HPointF &p1 = * (HPointF*) fTvor.Vertex(v1);
  HPointF &m  = * (HPointF*) fTvor.Vertex(fCurV);

  m = 0.5f * (p0 + p1);

  fEdgeMap.insert(make_pair(Edge(v0, v1), fCurV));

  // Edge e(v0,v1);
  // printf("  adding vertex %d for edge(%d, %d)\n", fCurV, e.v1, e.v2);

  return fCurV++;
}

Int_t HTriMesh::TringTvorSubdivider::SubdivideTriangle(Int_t t)
{
  // Returns triangle index of the first new triangle.

  // outer (old) and inner (new) triangles
  Int_t *o = fTvor.Triangle(t);
  Int_t  i[3] = {
    SubdivideEdge(o[0], o[1]),
    SubdivideEdge(o[1], o[2]),
    SubdivideEdge(o[2], o[0])
  };

  Int_t fti = fCurT;

  fTvor.SetTriangle(fCurT++, o[0], i[0], i[2]);
  fTvor.SetTriangle(fCurT++, i[0], o[1], i[1]);
  fTvor.SetTriangle(fCurT++, i[1], o[2], i[2]);
  fTvor.SetTriangle(fCurT++, i[2], i[0], i[1]);

  return fti;
}

void HTriMesh::TringTvorSubdivider::EndSubdivision()
{
  // Check if total n vertices/triangles matches with current counters.

  printf("HTriMesh::TringTvorSubdivider::EndSubdivision tring(%d,%d), vert(%d, %d)\n",
	 fCurT, fTvor.NTrings(), fCurV, fTvor.NVerts());
  assert(fCurT == fTvor.NTrings());
  assert(fCurV == fTvor.NVerts());
}
