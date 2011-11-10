// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GledTypes_H
#define GledCore_GledTypes_H

#include <Rtypes.h>
#include <TObject.h>
#include <TString.h>

#include <iostream>
#include <assert.h>

#include <vector>
#include <list>
#include <set>
#include <map>

#include <memory>
#include <exception>

#include <stdarg.h>

using namespace std;

#include <gled-config.h>

// String type and collections of TStrings
typedef list<TString>			lStr_t;
typedef list<TString>::iterator		lStr_i;
typedef list<TString>::const_iterator	lStr_ci;
typedef set<TString>			sStr_t;
typedef set<TString>::iterator		sStr_i;

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

typedef vector<ZGlass*>				vpZGlass_t;
typedef vector<ZGlass*>::iterator		vpZGlass_i;

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

/**************************************************************************/
// FID_t : Full libset/class ID & FMID_t: Full-method ID
/**************************************************************************/

class FID_t
{
public:
  LID_t	fLid;
  CID_t	fCid;

  FID_t(LID_t l=0, CID_t c=0) : fLid(l), fCid(c) {}
  virtual ~FID_t() {}
  bool operator==(FID_t r) const { return (fLid == r.fLid && fCid == r.fCid); }
  bool operator!=(FID_t r) const { return (fLid != r.fLid || fCid != r.fCid); }
  bool is_null()  const { return fLid == 0 && fCid == 0; }
  bool is_basic() const { return is_null() || (fLid == 1 && fCid == 1); }
  void clear() { fLid = 0; fCid = 0; }

  ClassDef(FID_t, 1);
};

namespace __gnu_cxx {
  template<>
  struct hash<FID_t>
  {
    size_t operator()(const FID_t& fid) const
    { size_t i = fid.fLid; i <<= 16; i |= fid.fCid; return i; }
  };
}

inline TBuffer &operator>>(TBuffer& b, FID_t& fid)
{ b >> fid.fLid >> fid.fCid; return b; }

inline TBuffer &operator<<(TBuffer& b, FID_t fid)
{ b << fid.fLid << fid.fCid; return b; }

class FMID_t : public FID_t
{
public:
  MID_t	fMid;

  FMID_t(LID_t l=0, CID_t c=0, MID_t m=0) : FID_t(l,c), fMid(m) {}
  virtual ~FMID_t() {}
  bool operator==(FMID_t r) { return FID_t::operator==(r) && fMid == r.fMid; }
  bool operator!=(FMID_t r) { return FID_t::operator!=(r) || fMid != r.fMid; }

  ClassDef(FMID_t, 1);
};

inline TBuffer &operator>>(TBuffer& b, FMID_t& fmid)
{ b >> (FID_t&)fmid >> fmid.fMid; return b; }

inline TBuffer &operator<<(TBuffer& b, FMID_t fmid)
{ b << (FID_t&)fmid << fmid.fMid; return b; }

/**************************************************************************/
// Defines
/**************************************************************************/

#define MAX_ID		((ID_t)(-1))
#define GLED_DEF_PORT	9061

/**************************************************************************/
// Exceptions, debug, warn, message stuff
/**************************************************************************/

class Exc_t : public std::exception, public TString
{
public:
   Exc_t() {}
   Exc_t(const TString& s) : TString(s) {}
   Exc_t(const char* s)    : TString(s) {}
   Exc_t(const std::string& s);

   virtual ~Exc_t() throw () {}

   virtual const char* what() const throw () { return Data(); }

   ClassDef(Exc_t, 1); // Exception-type thrown by Gled classes.
};

Exc_t operator+(const Exc_t &s1, const std::string& s2);
Exc_t operator+(const Exc_t &s1, const TString& s2);
Exc_t operator+(const Exc_t &s1, const char* s2);
Exc_t operator+(const Exc_t &s1,       char  c);

//==============================================================================

const char* GForm(const char* fmt, ...);
const char* GForm(const char* fmt, va_list args);

TBuffer& operator<<(TBuffer& b, const string& s);
TBuffer& operator>>(TBuffer& b, string& s);

bool operator==(const TString& t, const string& s);
bool operator==(const string&  s, const TString& t);

extern int G_DEBUG;

enum InfoStream_e { ISoutput, ISmessage, ISwarning, ISerror };
const char* const ISnames[]   = { "output", "message", "warning", "error" };
const char* const ISnames3c[] = { "OUT", "MSG", "WRN", "ERR" };
void InfoStream(InfoStream_e, const char* s);
void InfoStream(InfoStream_e, const TString& s);
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
