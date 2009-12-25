// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Audio1_AlContext_H
#define Audio1_AlContext_H

#include <Glasses/ZNode.h>

#ifndef __CINT__
#include <AL/alut.h>
#else
class ALCdevice;
class ALCcontext;
#endif

class AlContext : public ZNode
{
  MAC_RNR_FRIENDS(AlContext);

private:
  void _init();

protected:
  ALCdevice*  mDevice;  //! X{g}
  ALCcontext* mContext; //! X{g}

public:
  AlContext(const Text_t* n="AlContext", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  void Open();  // X{E}  7 MButt(-join=>1)
  void Close(); // X{E}  7 MButt()

#include "AlContext.h7"
  ClassDef(AlContext, 1);
}; // endclass AlContext


#endif
