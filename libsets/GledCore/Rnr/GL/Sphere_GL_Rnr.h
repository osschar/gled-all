// $#Header$
#ifndef Gled_Sphere_GL_Rnr_H
#define Gled_Sphere_GL_Rnr_H

#include <Glasses/Sphere.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <GL/gl.h>
#include <GL/glu.h>

class Sphere_GL_Rnr : public ZNode_GL_Rnr {
private:
  void _init();

protected:
  Sphere*	 mSphere;
  GLUquadricObj* mQuadric;

public:
  Sphere_GL_Rnr(Sphere* s) : ZNode_GL_Rnr(s), mSphere(s) { _init(); }
  virtual ~Sphere_GL_Rnr();

  virtual void Draw(RnrDriver* rd);

}; // endclass Sphere_GL_Rnr

#endif
