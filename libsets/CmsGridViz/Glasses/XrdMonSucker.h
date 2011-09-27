// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdMonSucker_H
#define CmsGridViz_XrdMonSucker_H

#include <Glasses/ZNameMap.h>
#include <Gled/GTime.h>

#include <TPRegexp.h>

class XrdServer;
class XrdFile;
class XrdFileCloseReporter;

class XrdMonSucker : public ZNameMap
{
  MAC_RNR_FRIENDS(XrdMonSucker);

private:
  void _init();

protected:
  Int_t      mSuckPort;     // X{GS} 7 Value()

  TString    mNagiosUser;   // X{GS} 7 Textor()
  TString    mNagiosHost;   // X{GS} 7 Textor()
  TString    mNagiosDomain; // X{GS} 7 Textor()

  TString    mTraceDN;      // X{GS} Ray{TraceRE} 7 Textor()
  TString    mTraceHost;    // X{GS} 7 Textor()
  TString    mTraceDomain;  // X{GS} 7 Textor()

  ZLink<XrdFileCloseReporter> mFCReporter; // X{GS} L{}

  Int_t      mSocket;       //!
  GThread   *mSuckerThread; //!

#ifndef __CINT__
  Bool_t     bTraceAllNull;   //!
  TPMERegexp mTraceDN_RE;     //!
  TPMERegexp mTraceHost_RE;   //!
  TPMERegexp mTraceDomain_RE; //!
#endif

#ifndef __CINT__
  struct xrdsrv_id
  {
    // This should actually be a stone ...

    UInt_t   ip4;
    Int_t    stod;
    UShort_t port;

    xrdsrv_id(UInt_t _ip4, Int_t _stod, UShort_t _port) :
      ip4(_ip4), stod(_stod), port(_port) {}

    bool operator==(const xrdsrv_id& si) const
    { return ip4 == si.ip4 && stod == si.stod && port == si.port; }

    struct hsh
    {
      // Initially the plan was to use CRC32 ... but this should be just as good.
      hash<size_t> hfoo;
      size_t operator()(const xrdsrv_id& si) const
      {	return hfoo((size_t) si.ip4 * (size_t) si.stod * (size_t) si.port); }
    };
  };

  typedef hash_map<xrdsrv_id, XrdServer*, xrdsrv_id::hsh> xrd_hash_t;
  typedef xrd_hash_t::iterator                            xrd_hash_i;

  xrd_hash_t    m_xrd_servers; //!
#endif

  void on_file_close(XrdFile* file);

  static void* tl_Suck(XrdMonSucker* s);
  void Suck();

public:
  XrdMonSucker(const Text_t* n="XrdMonSucker", const Text_t* t=0);
  virtual ~XrdMonSucker();

  void StartSucker(); // X{E} 7 MButt()
  void StopSucker();  // X{E} 7 MButt()

  void EmitTraceRERay();

#include "XrdMonSucker.h7"
  ClassDef(XrdMonSucker, 1);
}; // endclass XrdMonSucker

#endif
