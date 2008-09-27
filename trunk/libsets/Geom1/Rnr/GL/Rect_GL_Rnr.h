// $#Header$
#ifndef Geom1_Rect_GL_Rnr_H
#define Geom1_Rect_GL_Rnr_H

#include <Glasses/Rect.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

class Rect_GL_Rnr : public ZNode_GL_Rnr {
private:

protected:
  Rect*		mRect;

public:
  Rect_GL_Rnr(Rect* r) : ZNode_GL_Rnr(r), mRect(r) {}

  virtual void Draw(RnrDriver* rd);

}; // endclass Rect_GL_Rnr

#endif
