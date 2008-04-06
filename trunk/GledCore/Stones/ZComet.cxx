// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


//
// ZComet
//
// ZComet is a container class for streaming collections of glasses.
// It can be created from a ZCometBag, ZQueen or ZKing by calling the
// appropriate MakeComet() method.
// Service is rather elementary ... external references are only
// meaningfull if Saturn layout used for writing/reading is identical.
// A special class will be provided to make proper references into
// static objects.
// Comets of type CT_Queen & CT_King are used internally for exportation
// to connecting Moons.
//
// !!!!!! Need structure { ZGlass* g; bool links_done, lists_done; }
// as value type of idHash. Loops of infiniteness are awaiting you otherwise.
// Then, also, need no special treatment of ZNode::Parent. Or just maybe.

#include "ZComet.h"
#include <Gled/GledNS.h>
#include <Glasses/ZList.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZQueen.h>
#include <Ephra/Saturn.h>
#include <Gled/Gled.h>

#include <TBuffer.h>
#include <TDirectory.h>
#include <TFile.h>

ClassImp(ZComet)

/**************************************************************************/

void ZComet::_init()
{
  mType = CT_CometBag;
  mExtDemangler = 0;

  bWarnOn = false; bVerbose = false;
  mQueen = 0; mKing = 0;
}

/**************************************************************************/

Int_t ZComet::AddTopLevel(ZGlass* g, Bool_t do_links, Bool_t do_lists, Int_t depth)
{
  // !! should check if already in? not my problem ...
  mTopLevels.push_back(g);
  return AddGlass(g, do_links, do_lists, depth);
}

Int_t ZComet::AddGlass(ZGlass* g, Bool_t do_links, Bool_t do_lists, Int_t depth)
{
  // Adds a glass (et al) into local structure
  // depth controls traversal length
  // depth of 0 means add ONLY g, not even its links or lists.
  // depth of -1 means don't stop

  Int_t num_new = 0;
  ID_t id = g->GetSaturnID();
  if(mIDMap.find(id) == mIDMap.end()) {
    ++num_new;
    mIDMap[id] = g;
    mLibSets.insert(g->VFID().lid);
    ISdebug(D_STREAM, GForm("ZComet::AddGlass added lens %s, id=%u",
			    g->GetName(), id));
  }

  if(depth==0) return num_new;

  if(do_links) {
    ZGlass::lLinkRep_t lreps; g->CopyLinkReps(lreps);
    for(ZGlass::lLinkRep_i i = lreps.begin(); i != lreps.end(); ++i) {
      ZGlass* l = i->fLinkRef;
      if(l != 0 && mIgnoredLinks.find(i->fLinkInfo->FullName())==mIgnoredLinks.end()) {
	num_new += AddGlass(l, do_links, do_lists, depth-1);
      }
    }
  }

  ZList* l = dynamic_cast<ZList*>(g);
  if(l && do_lists) {
    lpZGlass_t members; l->CopyList(members);
    for(lpZGlass_i i=members.begin(); i!=members.end(); ++i) {
      num_new += AddGlass(*i, do_links, do_lists, depth-1);
    }
  }
  return num_new;
}

/**************************************************************************/

ZGlass* ZComet::DemangleID(ID_t id)
{
  static const Exc_t _eh("ZComet::DemangleID ");

  if(id == 0) return 0;
  mID2pZGlass_i i;
  if((i = mIDMap.find(id)) != mIDMap.end())
    return i->second;

  if(mExtDemangler != 0) {
    ZGlass* l = mExtDemangler->DemangleID(id);
    if(l == 0 && bVerbose)
      ISmess(_eh + GForm("id %u not found (comet, ext-demangler).", id));
    return l;
  } else {
    if(bVerbose)
      ISmess(_eh + GForm("id %u not found (comet).", id));
    return 0;
  }
}

/**************************************************************************/

void ZComet::AssignQueen(ZQueen* queen)
{
  // Sets the ZGlass::mQueen pointer to queen for all elements of the comet.
  // This should be called prior to RebuildGraph if it is desired that
  // reference counts are properly increased.

  for(mID2pZGlass_i i=mIDMap.begin(); i!=mIDMap.end(); i++) {
    i->second->mQueen = queen;
  }
}

Int_t ZComet::RebuildGraph()
{
  static const Exc_t _eh("ZComet::RebuildGraph ");

  if(bGraphRebuilt) return 0;
  Int_t ret = 0;

  // First pass: rebuild and count missed links & list members
  for(mID2pZGlass_i i=mIDMap.begin(); i!=mIDMap.end(); i++) {
    ZGlass* g = i->second;
    if(Int_t m = g->RebuildLinkRefs(this)) {
      ret += m;
      if(bWarnOn)
	ISwarn(_eh + GForm("(links) %d missed lens(es) in '%s'.",
			   m, g->GetName()));
    }
    ZList* l = dynamic_cast<ZList*>(g);
    if(l) {
      Int_t m = l->RebuildListRefs(this);
      if(m>0) {
	ret += m;
	if(bWarnOn)
	  ISwarn(_eh + GForm("(list) %d missed lens(es) in '%s'.",
			     m, g->GetName()));
      }
    }
  }

  // Second pass: grep NotRefed Glasses that are NOT toplevel
  // !!!! hmmph ... should copy map, and remove connected ones.
  for(mID2pZGlass_i i=mIDMap.begin(); i!=mIDMap.end(); ++i) {
    ZGlass* g = i->second;
    if(g->GetRefCount()==0) {
      if(find(mTopLevels.begin(), mTopLevels.end(), g) == mTopLevels.end()) {
	ISdebug(D_STREAM, _eh + GForm("'%s' is an orphan.", g->GetName()));
	mOrphans.push_back(g);
      }
    }
  }
  bGraphRebuilt = true;
  return ret;
}

/**************************************************************************/
/**************************************************************************/
// Streamer functions
/**************************************************************************/
/**************************************************************************/

void ZComet::Streamer(TBuffer& b)
{
  static const Exc_t _eh("ZComet::Streamer ");

  StreamHeader(b);
  StreamContents(b);

  if(b.IsReading()) {
    // Rebuild kings/queens
    if(mType == CT_Queen) {
      ID_t qid = GledNS::CastLens2ID(mQueen);
      mQueen = dynamic_cast<ZQueen*>(DemangleID(qid));
      if(mQueen==0) {
	ISerr(_eh + GForm("(Queen) couldn't demangle QueenID %u.", qid));
      }
    }
    if(mType == CT_King) {
      ID_t kid = GledNS::CastLens2ID(mKing);
      mKing = dynamic_cast<ZKing*>(DemangleID(kid));
      if(mKing==0) {
	ISerr(_eh + GForm("(King) couldn't demangle KingID %u.", kid));
      }
    }
    // Reconstruct TopLevels
    for(lpZGlass_i i=mTopLevels.begin(); i!=mTopLevels.end(); ++i) {
      ID_t id = GledNS::CastLens2ID(*i);
      mID2pZGlass_i j = mIDMap.find(id);
      if(j != mIDMap.end()) {
	*i = j->second;
      } else {
	if(bWarnOn)
	  ISwarn(_eh + GForm("(top_levels) missing ID %u.", id));
      }
    }
  }
}

void ZComet::StreamHeader(TBuffer& b)
{
  // Streams Type, Size, Libsets, TopLevels|Queen|King

  static const Exc_t _eh("ZComet::StreamHeader ");

  if(b.IsReading()) {
    /*** Reading ***/
    bGraphRebuilt = false;
    mLibSets.clear(); mTopLevels.clear(); mOrphans.clear();

    UInt_t t; b >> t; mType = CometType_e(t);

    UInt_t cnt; b >> cnt;
    bFail = false;
    for(UInt_t i=0; i<cnt; i++) {
      LID_t lid; b >> lid;
      mLibSets.insert(lid);
      if(!GledNS::IsLoaded(lid)) {
	ISmess(_eh + GForm("attempting to load LibSet w/ id=%u.", lid));
	int ret = Gled::theOne->LoadLibSet(lid);
	if(ret != 0) {
	  ISerr(_eh + GForm("failed to load LibSet w/ id=%u.", lid));
	  bFail = true;
	}
      }
    }
    if(bFail) throw _eh + "LibSets missing.";

    switch(mType) {
    case CT_CometBag:
      b >> cnt;
      for(UInt_t i=0; i<cnt; i++) {
	mTopLevels.push_back(GledNS::ReadLensIDAsPtr(b));
      }
      break;
    case CT_Queen: {
      mQueen = (ZQueen*) GledNS::ReadLensIDAsPtr(b);
      break;
    }
    case CT_King: {
      mKing  = (ZKing*)  GledNS::ReadLensIDAsPtr(b);
      break;
    }
    }

  } else {
    /*** Writing ***/
    b << (UInt_t) mType;
    b << (UInt_t) mLibSets.size();
    for(set<LID_t>::iterator i=mLibSets.begin(); i!=mLibSets.end(); ++i)
      b << *i;
    switch(mType) {
    case CT_CometBag:
      b << (UInt_t) mTopLevels.size();
      for(lpZGlass_i i=mTopLevels.begin(); i!=mTopLevels.end(); ++i)
	GledNS::WriteLensID(b, *i);
      break;
    case CT_Queen: GledNS::WriteLensID(b, mQueen); break;
    case CT_King:  GledNS::WriteLensID(b, mKing);  break;
    }
  }
}

void ZComet::StreamContents(TBuffer& b)
{
  static const Exc_t _eh("ZComet::StreamContents ");

  if(b.IsReading()) {
    UInt_t size;
    b >> size;
    ISdebug(D_STREAM, _eh + GForm("reading %u glasses.", size));
    for(UInt_t i=0; i<size; ++i) {
      ZGlass *g = GledNS::StreamLens(b);
      if(g) {
	ID_t id = g->GetSaturnID();
	mIDMap[id] = g;
	ISdebug(D_STREAM+1, _eh + GForm("read lens %s[%s] addr=%p.",
				  g->GetName(), g->ClassName(), (void*)g));
      } else {
	ISdebug(D_STREAM, _eh + "lens creation failed.");
      }
    }
  } else {
    UInt_t size = (UInt_t)(mIDMap.size());
    b << size;
    ISdebug(D_STREAM, _eh + GForm("writing %u glasses.", size));
    if(size > 0) {
      for(mID2pZGlass_i i=mIDMap.begin(); i!=mIDMap.end(); i++) {
	ISdebug(D_STREAM+1, _eh + GForm("writing %s", (i->second)->GetName()));
	GledNS::StreamLens(b, i->second);
      }
    }
  }
}

/**************************************************************************/
