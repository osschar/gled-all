// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdEhs_H
#define CmsGridViz_XrdEhs_H

#include <Glasses/ZNameMap.h>


class XrdMonSucker;

class XrdEhs : public ZNameMap
{
private:
   void _init();

   Bool_t	        b_stop_server; //!

protected:
   ZLink<XrdMonSucker>  mXrdSucker;    // X{GS} L{a}
   Int_t	        mPort;         // X{GS} 7 Value(-range=>[1,65535,1])
   Bool_t	        bServerUp;     // X{GS} 7 BoolOut()

public:
   XrdEhs(const Text_t* n="XrdEhs", const Text_t* t=0);
   virtual ~XrdEhs();

   void StartServer(); // X{Ed} 7 MButt(-join=>1)
   void StopServer();  // X{E}  7 MButt()

#include "XrdEhs.h7"
   ClassDef(XrdEhs, 1);
}; // endclass XrdEhs

#endif
