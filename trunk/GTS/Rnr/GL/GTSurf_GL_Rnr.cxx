// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTSurf_GL_Rnr.h"
#include <GTS/GTS.h>
#include <GL/gl.h>

/**************************************************************************/

namespace {

  int face_drawer(GTS::GtsFace* f, int* dum) {
    /*
      GtsPoint& p1 = t->e1->segment.v1->p;
      GtsPoint& p2 = t->e2->segment.v2->p;
      glVertex3d(p1.x, p1.y, p1.z);
      glVertex3d(p2.x, p2.y, p2.z);
    */
    GTS::gdouble     n[3];
    GTS::GtsVertex* vp[3];
    GTS::gts_triangle_normal(&f->triangle, &n[0], &n[1], &n[2]);
    glNormal3dv(n);
    GTS::gts_triangle_vertices(&f->triangle, &vp[0], &vp[1], &vp[2]);
    glVertex3dv(&vp[0]->p.x);
    glVertex3dv(&vp[1]->p.x);
    glVertex3dv(&vp[2]->p.x);
    return 0;
  }

}

void GTSurf_GL_Rnr::Render(RnrDriver* rd)
{
  glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);

  glColor4fv(mGTSurf->mColor());
  if(mGTSurf->pSurf) {
    glPushMatrix();
    glScalef(mGTSurf->mScale, mGTSurf->mScale, mGTSurf->mScale);
    glEnable(GL_NORMALIZE);
    glBegin(GL_TRIANGLES);
    GTS::gts_surface_foreach_face(mGTSurf->pSurf, (GTS::GtsFunc)face_drawer, 0);
    glEnd();
    glPopMatrix();
  }

  glPopAttrib();
}
