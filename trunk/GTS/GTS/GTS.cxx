// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTS.h"
#include <Gled/GledTypes.h>
#include <FL/gl.h>
#include <iostream>

namespace GTS {

  void vertex_dumper(GtsVertex* v) {
    printf("(%4.2f,%4.2f,%4.2f)", v->p.x, v->p.y, v->p.z);
  }

  void segment_dumper(GtsSegment* s) {
    vertex_dumper(s->v1); cout<<"-"; vertex_dumper(s->v2);
  }


  void triangle_dumper(GtsTriangle* t, int* n) {
    cout <<"Tring #"<< *n <<"\t";
    segment_dumper(&t->e1->segment); cout<<" . ";
    segment_dumper(&t->e2->segment); cout<<" . ";
    segment_dumper(&t->e3->segment); cout<<"\n";
    ++(*n);
  }

  void strip_dumper(GSList* tl, int* n) {
    cout <<"Strip #"<< *n <<endl;
    int num = 0;
    g_slist_foreach(tl, (GFunc)triangle_dumper, &num);
    ++(*n);
  }


  int face_inverter(GtsFace* f, int* dum) {
    GtsEdge* egg = f->triangle.e1;
    f->triangle.e1 = f->triangle.e2;
    f->triangle.e2 = egg;
    return 0;
  }
} // gts slurping namespace

/**************************************************************************/
/**************************************************************************/

GTS::Surface::~Surface() {
  if(surface) {
    // Edges remaining ... some global var controls that
    gts_object_destroy (GTS_OBJECT (surface));
  }
}

GTS::Surface* GTS::Surface::Load(const TString& gtl)
{
  FILE* fp = fopen(gtl.Data(), "r");
  if(!fp) {
    ISerr(GForm("GTS::Surface::Load Cant' open %s", gtl.Data()));
    return 0;
  }
  GtsSurface* s = gts_surface_new(gts_surface_class (),
				  gts_face_class (),
				  gts_edge_class (),
				  gts_vertex_class ());
  if(s==0) {
    ISerr(GForm("GTS::Surface::Load gts_surface_new failed ..."));
    return 0;
  }
  GtsFile* gsf = gts_file_new(fp);
  if( gts_surface_read(s, gsf) != 0 ) {
    // should destroy surface
    ISerr(GForm("GTS::Surface::Load gts_surface_read failed ..."));
    return 0;
  }
  gts_file_destroy(gsf);
  fclose(fp);
  gts_surface_print_stats(s, stdout);
  Surface* ms = new Surface;
  ms->surface = s;
  return ms;
}

void GTS::Surface::Invert()
{
  gts_surface_foreach_face(surface, (GtsFunc)face_inverter, 0);
}
