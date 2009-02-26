// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_SphereTrings
#define Gled_SphereTrings

#include <GL/glew.h>

namespace SphereTrings {
  extern GLfloat  CubeA;
  extern GLfloat  OctusA;

  extern GLfloat *Vertexen[5];
  extern GLfloat *Normaleen[5];
  extern GLubyte *Indexen[5];
  extern GLsizei  IndexSize[5];
  extern GLenum   GLmode[5];

  void Render(int i, bool flat_p);

  void EnableGL(int i);
  void DrawAndDisableGL(int i);

  void UnitBox();
  void UnitFrameBox();

} // namespace SphereTrings

#endif
