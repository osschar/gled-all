// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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
#include <Ephra/Forest.h>
#include <Ephra/Mountain.h>
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

  bWarnOn = true; bVerbose = true;
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
    mLibSets.insert(g->ZibID());
    ISdebug(D_STREAM, GForm("ZComet::AddGlass added lens %s, id=%u",
			    g->GetName(), id));
  }

  if(depth==0) return num_new;

  if(do_links) {
    lpZGlass_t  links;  g->CopyLinks(links);	  lpZGlass_i  i=links.begin();
    lLinkSpec_t lspecs; g->CopyLinkSpecs(lspecs); lLinkSpec_i j=lspecs.begin();
    while(i!=links.end()) {
      if(*i != 0 && mIgnoredLinks.find(j->full_name())==mIgnoredLinks.end()) {
	num_new += AddGlass(*i, do_links, do_lists, depth-1);
      }
      ++i; ++j;
    }
  }

  ZList* l = dynamic_cast<ZList*>(g);
  if(l && do_lists) {
    lpZGlass_t members; l->Copy(members);
    for(lpZGlass_i i=members.begin(); i!=members.end(); ++i) {
      num_new += AddGlass(*i, do_links, do_lists, depth-1);
    }
  }
  return num_new;
}

/**************************************************************************/

ZGlass* ZComet::DemangleID(ID_t id)
{
  static const string _eh("ZComet::DemangleID ");

  if(id == 0) return 0;
  mID2pZGlass_i i;
  if((i = mIDMap.find(id)) != mIDMap.end()) 
    return i->second;

  if(mExtDemangler != 0) {
    ZGlass* l = mExtDemangler->DemangleID(id);
    if(l == 0 && bVerbose) 
      ISmess(GForm("%sid %u not found neither in this comet nor saturn.", _eh.c_str(), id));
    return l;
  } else {
    if(bVerbose)
      ISmess(GForm("%sid %u not found in this comet.", _eh.c_str(), id));
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
  if(bGraphRebuilt) return 0;
  Int_t ret = 0;

  // First pass: rebuild and count missed links & list members
  for(mID2pZGlass_i i=mIDMap.begin(); i!=mIDMap.end(); i++) {
    ZGlass* g = i->second;
    if(Int_t m = g->RebuildLinkRefs(this)) {
      ret += m;
      if(bWarnOn)
	ISwarn(GForm("ZComet::RebuildGraph(links) %d missed lens(es) in '%s'.",
		     m, g->GetName()));
    }
    ZList* l = dynamic_cast<ZList*>(g);
    if(l) {
      Int_t m = l->RebuildListRefs(this);
      if(m>0) {
	ret += m;
	if(bWarnOn)
	  ISwarn(GForm("ZComet::RebuildGraph(list) %d missed lens(es) in '%s'.",
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
	ISdebug(D_STREAM, GForm("ZComet::RebuildGraph '%s' is an orphan.",
				g->GetName()));
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
  static const string _eh("ZComet::Streamer ");

  StreamHeader(b);
  StreamContents(b);

  if(b.IsReading()) {
    // Rebuild kings/queens
    if(mType == CT_Queen) {
      mQueen = dynamic_cast<ZQueen*>(DemangleID((ID_t)mQueen));
      if(mQueen==0) {
	ISerr(GForm("%s(Queen) couldn't demangle QueenID %u",
		    _eh.c_str(), (ID_t)(mQueen)));
      }
    }
    if(mType == CT_King) {
      mKing = dynamic_cast<ZKing*>(DemangleID((ID_t)mKing));
      if(mKing==0) {
	ISerr(GForm("%s(King) couldn't demangle KingID %u",
		    _eh.c_str(), (ID_t)(mKing)));
      }
    }
    // Reconstruct TopLevels
    for(lpZGlass_i i=mTopLevels.begin(); i!=mTopLevels.end(); ++i) {
      mID2pZGlass_i j = mIDMap.find((ID_t)(*i));
      if(j != mIDMap.end()) {
	*i = j->second;
      } else {
	if(bWarnOn)
	  ISwarn(GForm("%s(top_levels) missing ID %u",
		       _eh.c_str(), (ID_t)(*i)));
      }
    }
  }
}

void ZComet::StreamHeader(TBuffer& b)
{
  // Streams Type, Size, Libsets, TopLevels|Queen|King
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
	ISwarn(GForm("ZComet::Streamer attempting to load LibSet w/ id=%u", lid));
	int ret = Gled::theOne->LoadLibSet(lid);
	if(ret != 0) {
	  ISerr(GForm("ZComet::Streamer failed to load LibSet w/ id=%u", lid));
	  bFail = true;
	}
      }
    }
    if(bFail) throw(string("LibSets missing"));

    switch(mType) {
    case CT_CometBag:
      b >> cnt;
      for(UInt_t i=0; i<cnt; i++) {
	ID_t id; b >> id;
	mTopLevels.push_back((ZGlass*)id);
      }
      break;
    case CT_Queen: b >> mQueen; break;
    case CT_King:  b >> mKing;  break;
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
	b << (*i)->GetSaturnID();
      break;
    case CT_Queen: b << mQueen; break;
    case CT_King:  b << mKing;  break;
    }
  }
}

void ZComet::StreamContents(TBuffer& b)
{
  if(b.IsReading()) {
    UInt_t size;
    b >> size;
    ISdebug(D_STREAM, GForm("ZComet::Streamer reading %u glasses", size));
    for(UInt_t i=0; i<size; ++i) {
      ZGlass *g = GledNS::StreamLens(b);
      if(g) {
	ID_t id = g->GetSaturnID();
	mIDMap[id] = g;
	ISdebug(D_STREAM+1, GForm("ZComet::Streamer read glass %s[%s] addr=%p",
				g->GetName(), g->ClassName(), (void*)g));
      } else {
	ISdebug(D_STREAM, GForm("ZComet::Streamer failed creating node with lid=%u cid=%u",
				g->ZibID(), g->ZlassID()));
      }
    }
  } else {
    UInt_t size = (UInt_t)(mIDMap.size());
    b << size;
    ISdebug(D_STREAM, GForm("ZComet::Streamer writing %u glasses", size));
    if(size > 0) {
      for(mID2pZGlass_i i=mIDMap.begin(); i!=mIDMap.end(); i++) {
	ISdebug(D_STREAM+1, GForm("ZComet::Streamer writing %s",
				(i->second)->GetName()));
	GledNS::StreamLens(b, i->second);
      }
    }
  }
}

/**************************************************************************/
