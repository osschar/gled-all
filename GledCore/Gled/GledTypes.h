// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GledTypes_H
#define Gled_GledTypes_H

#include <Rtypes.h>
#include <iostream>
#include <string>
#include <assert.h>

#include <list>
#include <set>
#include <map>

using namespace std;

#include <config.h>

// String type and collections of strings
typedef string				Str_t;
typedef string::iterator		Str_i;
typedef string::const_iterator		Str_ci;
typedef list<string>			lStr_t;
typedef list<string>::iterator		lStr_i;
typedef list<string>::const_iterator	lStr_ci;
typedef set<string>			sStr_t;
typedef set<string>::iterator		sStr_i;

/**************************************************************************/
// Glass typedefs
/**************************************************************************/

class ZGlass;
typedef list<ZGlass*>			lpZGlass_t;
typedef list<ZGlass*>::iterator		lpZGlass_i;
typedef list<ZGlass*>::const_iterator	lpZGlass_ci;
typedef list<ZGlass*>::reverse_iterator	lpZGlass_ri;
typedef list<ZGlass**>			lppZGlass_t;
typedef list<ZGlass**>::iterator	lppZGlass_i;

typedef UInt_t		ID_t;
typedef UShort_t	LID_t;
typedef UShort_t	CID_t;
typedef UShort_t	MID_t;

struct FID_t { // Full lib/class ID
  LID_t	lid;
  CID_t	cid;
  FID_t(LID_t l, CID_t c) : lid(l), cid(c) {}
  bool operator==(FID_t& r) { return (lid==r.lid && cid==r.cid); }
  bool is_null() { return lid==0 && cid==0; }
};

struct LinkSpec {
  string fClassName;
  string fLinkName;
  LinkSpec(const char* c, const char* l) : fClassName(c), fLinkName(l) {}
  string full_name() { return fClassName + "::" + fLinkName; }
};

typedef list<LinkSpec>			lLinkSpec_t;
typedef list<LinkSpec>::iterator	lLinkSpec_i;

typedef ULong_t		TimeStamp_t;
typedef UInt_t		UCIndex_t;	// Unckecked Index for ZVec/Mat
typedef Int_t		xxIndex_t;	// debug index type

typedef	list<ID_t>		lID_t;
typedef	list<ID_t>::iterator	lID_i;
typedef set<ID_t>		sID_t;
typedef set<ID_t>::iterator	sID_i;
#ifndef __CINT__
typedef map<ID_t, ZGlass*>			mID2pZGlass_t;
typedef map<ID_t, ZGlass*>::iterator		mID2pZGlass_i;
typedef hash_map<ID_t, ZGlass*>			hID2pZGlass_t;
typedef hash_map<ID_t, ZGlass*>::iterator	hID2pZGlass_i;
typedef hash_map<ID_t, ID_t>			IdiOm_t;
typedef hash_map<ID_t, ID_t>::iterator		IdiOm_i;
#endif

/**************************************************************************/
// Defines
/**************************************************************************/

#define MAX_ID		((ID_t)(-1))
#define GLED_DEF_PORT	9061

/**************************************************************************/
// Debug, warn, mess Stuff
/**************************************************************************/

const char* GForm(const char* fmt, ...);

extern int G_DEBUG;

enum InfoStream_e { ISoutput, ISmessage, ISwarning, ISerror };
void InfoStream(InfoStream_e, const char* s);
void InfoStream(InfoStream_e, const string& s);

#define ISout(_str_)  { InfoStream(ISoutput, _str_); }
#define ISmess(_str_) { InfoStream(ISmessage, _str_); }
#define ISwarn(_str_) { InfoStream(ISwarning, _str_); }
#define ISerr(_str_)  { InfoStream(ISerror, _str_); }

#define D_STREAM	4
#define D_THRMUT	2

#ifdef DEBUG
#define ISdebug(A,B) 	{ if((A)<=G_DEBUG) ISmess(B) }
#else
#define ISdebug(A,B)
#endif

#endif
