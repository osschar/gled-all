// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef LIBSETNAME_CLASSNAME_H
#define LIBSETNAME_CLASSNAME_H

#include <Glasses/BASENAME.h>

class CLASSNAME : public BASENAME {
  MAC_RNR_FRIENDS(CLASSNAME);

private:
  void _init();

protected:

public:
  CLASSNAME(const Text_t* n="CLASSNAME", const Text_t* t=0) :
    BASENAME(n,t) { _init(); }


#include "CLASSNAME.h7"
  ClassDef(CLASSNAME, 1)
}; // endclass CLASSNAME

GlassIODef(CLASSNAME);

#endif
