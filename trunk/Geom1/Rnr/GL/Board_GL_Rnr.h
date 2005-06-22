// $#Header$
#ifndef Geom1_Board_GL_Rnr_H
#define Geom1_Board_GL_Rnr_H

#include <Glasses/Board.h>
#include <Rnr/GL/ZNode_GL_Rnr.h>

#include <GL/gl.h>

class Board_GL_Rnr : public ZNode_GL_Rnr {
private:

protected:
  Board*	mBoard;
  GLuint	mTexture;

public:
  Board_GL_Rnr(Board* b) : ZNode_GL_Rnr(b), mBoard(b), mTexture(0) {}
  virtual ~Board_GL_Rnr();

  virtual void Draw(RnrDriver* rd);
}; // endclass Board_GL_Rnr

#endif
