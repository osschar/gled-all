// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ShellInfo
//
// Glass representation of GUI shell 'FTW_Shell'.
// Note that FTW_Shell itself is subclassed from Eye.
// ShellInfo can not subclass EyeInfo as it is instantiated on the
// Sun Absolute.
// In principle should have a link to ;swallowyesed view and display a canvas
// of some sort if it is not set.

#include "ShellInfo.h"
#include "MetaViewInfo.h"
#include "ShellInfo.c7"
#include "NestInfo.h"
#include <Glasses/ZQueen.h>

ClassImp(ShellInfo)

/**************************************************************************/

void ShellInfo::_init()
{
  mSubShells   = 0;
  mDefSubShell = 0;  
  mBeta = mGamma = 0;
  mMessageRecipient = 0;

  mDefW       = 80;
  mDefSShellH = 28;
  bDefSourceVis = true;
  bDefSinkVis   = false;

  mMsgOutH     = 5;

  bFancyClassView       = true; bCollZGlass = bCollZList = true;
  bShowLinksInClassView = true;
}

/**************************************************************************/

void ShellInfo::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mSubShells == 0) {
    ZHashList* l = new ZHashList("SubShells", GForm("SubShells of %s", GetName()));
    l->SetMIRActive(false);
    mQueen->CheckIn(l);
    SetSubShells(l);
  }
  const TString etc("Etc");
  if(!GetElementByName(etc)) {
    ZNameMap* nm = new ZNameMap(etc.Data());
    mQueen->CheckIn(nm); Add(nm);
  }
}

/**************************************************************************/

SubShellInfo* ShellInfo::MakeDefSubShell()
{
  NestInfo* nest = new NestInfo("Default Nest");
  mQueen->CheckIn(nest);
  nest->ImportKings();
  AddSubShell(nest);
  SetDefSubShell(nest);
  return nest;
}

/**************************************************************************/

void ShellInfo::AddSubShell(SubShellInfo* sub_shell)
{
  mSubShells->Add(sub_shell);
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_add_subshell, mTimeStamp, FID()));
    ray->SetBeta(sub_shell);
    mQueen->EmitRay(ray);
  }
}

void ShellInfo::RemoveSubShell(SubShellInfo* sub_shell)
{
  mSubShells->Remove(sub_shell);
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_remove_subshell, mTimeStamp, FID()));
    ray->SetBeta(sub_shell);
    mQueen->EmitRay(ray);
  }
}

/**************************************************************************/

void ShellInfo::SetDefSubShell(SubShellInfo* ss)
{
  static const Exc_t _eh("ShellInfo::SetDefSubShell ");
  if(!mSubShells->Has(ss)) {
    AddSubShell(ss);
  }
  set_link_or_die(mDefSubShell.ref_link(), ss, FID());
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_set_def_subshell, mTimeStamp, FID()));
    ray->SetBeta(ss);
    mQueen->EmitRay(ray);
  }
}

void ShellInfo::SetSubShells(ZHashList* ss)
{
  static const Exc_t _eh("ShellInfo::SetSubShells ");
  if(mSubShells != 0)
    throw(_eh + "can not be changed.");
  set_link_or_die(mSubShells.ref_link(), ss, FID());
}

/**************************************************************************/

void ShellInfo::SpawnMetaGui(ZGlass* source, MetaViewInfo* gui_template)
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_spawn_metagui, mTimeStamp, FID()));
    ray->SetBeta(source);
    ray->SetGamma(gui_template);
    mQueen->EmitRay(ray);
  }
}

/**************************************************************************/

void ShellInfo::EmitResizeRay()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_resize_window, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}
