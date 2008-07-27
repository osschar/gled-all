// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef LIBSETNAME_CLASSNAME_H
#define LIBSETNAME_CLASSNAME_H

#include <Stones/BASENAME.h>

class CLASSNAME : public BASENAME {

private:
  void _init();

protected:

public:
  CLASSNAME(const Text_t* n="CLASSNAME", const Text_t* t=0) :
    BASENAME(n,t)
  { _init(); }


#include "CLASSNAME.h7"
  ClassDef(CLASSNAME, 1);
}; // endclass CLASSNAME

#endif
