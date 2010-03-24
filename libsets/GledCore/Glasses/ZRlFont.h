// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlFont_H
#define GledCore_ZRlFont_H

#include <Glasses/ZRnrModBase.h>
#include <Eye/Ray.h>

class ZRlFont : public ZRnrModBase
{
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZRlFont);

public:
  enum FontMode_e { FM_Bitmap,  FM_Pixmap,  FM_Texture,
		    FM_Outline, FM_Polygon, FM_Extrude };	

private:
  enum PrivRayQN_e  { PRQN_offset = RayNS::RQN_user_0,
		      PRQN_font_change,
		      PRQN_size_change,
		      PRQN_depth_change		      
  };

  void _init();

protected:
  FontMode_e    mMode;        // X{GST} Ray{FontChange}  7 PhonyEnum()
  TString	mFontFile;    // X{GET}                  7 Filor(-pat=>"*.ttf")
  Int_t         mSize;        // X{GST} Ray{SizeChange}  7 Value(-range=>[4,512,1])
  Float_t       mDepthFac;    // X{GST} Ray{DepthChange} 7 Value(-range=>[0,10, 1,100])

public:
  ZRlFont(const Text_t* n="ZRlFont", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

  void SetFontFile(const Text_t* f);

  void EmitFontChangeRay();
  void EmitSizeChangeRay();
  void EmitDepthChangeRay();

#include "ZRlFont.h7"
  ClassDef(ZRlFont, 1);
}; // endclass ZRlFont


#endif
