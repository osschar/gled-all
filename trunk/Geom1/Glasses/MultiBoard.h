// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_MultiBoard_H
#define Geom1_MultiBoard_H

#include <Glasses/Board.h>
#include <Glasses/ZHashList.h>

class MultiBoard : public Board {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(MultiBoard);

private:
  void _init();

protected:
  ZHashList*	mSlides;	// X{gS} L{}

public:
  MultiBoard(const Text_t* n="MultiBoard", const Text_t* t=0) :
    Board(n,t) { _init(); }

  virtual void AdEnlightenment();

  void First(); // X{E} 7 MButt(-join=>1)
  void Last();  // X{E} 7 MButt()
  void Prev();  // X{E} 7 MButt(-join=>1)
  void Next();  // X{E} 7 MButt()

#include "MultiBoard.h7"
  ClassDef(MultiBoard, 1) // Board with a sequence of images/slides.
}; // endclass MultiBoard

GlassIODef(MultiBoard);

#endif
