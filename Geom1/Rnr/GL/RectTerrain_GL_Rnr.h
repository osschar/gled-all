// $#Header$
#ifndef Geom1_RectTerrain_GL_Rnr_H
#define Geom1_RectTerrain_GL_Rnr_H

#include <Glasses/RectTerrain.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>
#include <FL/gl.h>

class RectTerrain_GL_Rnr : public ZNode_GL_Rnr {
private:
  void check(double,double,GLdouble*,GLdouble*,GLint*);

protected:
  RectTerrain*		mTerrain;

  void MkVN(UCIndex_t i, UCIndex_t j);

public:
  RectTerrain_GL_Rnr(RectTerrain* rt);

  virtual void Draw(RnrDriver* rd);

}; // endclass RectTerrain_GL_Rnr

#endif
