// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GledTypes_H
#define GledCore_GledTypes_H

#include <Rtypes.h>
#include <TObject.h>
#include <TString.h>

#include <iostream>
#include <string>
#include <assert.h>

#include <list>
#include <set>
#include <map>

#include <memory>

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

typedef ULong_t				TimeStamp_t;

/**************************************************************************/
// Glass typedefs
/**************************************************************************/

class ZGlass;
typedef list<ZGlass*>				lpZGlass_t;
typedef list<ZGlass*>::iterator			lpZGlass_i;
typedef list<ZGlass*>::const_iterator		lpZGlass_ci;
typedef list<ZGlass*>::reverse_iterator		lpZGlass_ri;
typedef list<ZGlass*>::const_reverse_iterator	lpZGlass_cri;
typedef list<ZGlass**>				lppZGlass_t;
typedef list<ZGlass**>::iterator		lppZGlass_i;

typedef UInt_t					ID_t;
typedef UShort_t				LID_t;
typedef UShort_t				CID_t;
typedef UShort_t				MID_t;

typedef	list<ID_t>				lID_t;
typedef	list<ID_t>::iterator			lID_i;
typedef set<ID_t>				sID_t;
typedef set<ID_t>::iterator			sID_i;

#ifndef __CINT__

typedef map<ID_t, ZGlass*>			mID2pZGlass_t;
typedef map<ID_t, ZGlass*>::iterator		mID2pZGlass_i;
typedef hash_map<ID_t, ZGlass*>			hID2pZGlass_t;
typedef hash_map<ID_t, ZGlass*>::iterator	hID2pZGlass_i;
typedef hash_map<ZGlass*, Int_t>		hpZGlass2Int_t;
typedef hash_map<ZGlass*, Int_t>::iterator	hpZGlass2Int_i;
typedef hash_map<ID_t, ID_t>			IdiOm_t;
typedef hash_map<ID_t, ID_t>::iterator		IdiOm_i;

#endif

class An_ID_Demangler { public: virtual ZGlass* DemangleID(ID_t) = 0; };

/**************************************************************************/
// FID_t : Full libset/class ID & FMID_t: Full-method ID
/**************************************************************************/

class FID_t {
public:
  LID_t	lid;
  CID_t	cid;

  FID_t(LID_t l=0, CID_t c=0) : lid(l), cid(c) {}
  bool operator==(FID_t r) const { return (lid == r.lid && cid == r.cid); }
  bool operator!=(FID_t r) const { return (lid != r.lid || cid != r.cid); }
  bool is_null()  const { return lid == 0 && cid == 0; }
  bool is_basic() const { return is_null() || (lid == 1 && cid == 1); }
  void clear() { lid = 0; cid = 0; }
};

namespace __gnu_cxx {
  template<>
  struct hash<FID_t> {
    size_t operator()(const FID_t& fid) const
    { size_t i = fid.lid; i <<= 16; i |= fid.cid; return i; }
  };
}

inline TBuffer &operator>>(TBuffer& b, FID_t& fid)
{ b >> fid.lid >> fid.cid; return b; }

inline TBuffer &operator<<(TBuffer& b, FID_t fid)
{ b << fid.lid << fid.cid; return b; }

class FMID_t : public FID_t {
public:
  MID_t	mid;

  FMID_t(LID_t l=0, CID_t c=0, MID_t m=0) : FID_t(l,c), mid(m) {}
  bool operator==(FMID_t r) { return FID_t::operator==(r) && mid == r.mid; }
  bool operator!=(FMID_t r) { return FID_t::operator!=(r) || mid != r.mid; }
};

inline TBuffer &operator>>(TBuffer& b, FMID_t& fmid)
{ b >> (FID_t&)fmid >> fmid.mid; return b; }

inline TBuffer &operator<<(TBuffer& b, FMID_t fmid)
{ b << (FID_t&)fmid << fmid.mid; return b; }

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
