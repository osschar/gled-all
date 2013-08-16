// $Id: Stone_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Net1_SNetResolver_H
#define Net1_SNetResolver_H

#include <TPRegexp.h>

class SUdpPacket;


class SNetResolver
{
private:
  void _init();

protected:
  TPMERegexp    m_hostname_re;
  TPMERegexp    m_ip4_numeric_re;
  TPMERegexp    m_ip4_in_name_re;

public:
  // Result members ... really?
  // TString       host;
  // TString       domain;
  // Bool_t        is_numeric;
  // Bool_t        is_local;
  // Bool_t        is_known;
  //
  // void reset_result_members()
  //{
  //  host = domain = "";
  //  // WTF with the bools ... this is a bad idea
  //}

  // TString    info; ??


public:
  SNetResolver();
  virtual ~SNetResolver();

  Bool_t is_numeric(const TString& hostname);
  Bool_t was_local();

  Bool_t resolve_fqhn
  (SUdpPacket *p,
   TString &fqhn,
   Bool_t *numeric_p=0);

  Bool_t split_non_numeric_fqhn_to_host_domain
  (
   const TString &fqhn,
   TString &host, TString &domain);

  Bool_t split_fqhn_to_host_domain_no_lookup
  (
   const TString &fqhn,
   TString &host, TString &domain,
   Bool_t *numeric_p=0);

  Bool_t split_fqhn_to_host_domain_with_lookup
  (
   const TString& fqhn_in,
   TString &host, TString &domain,
   Bool_t *numeric_p=0);

#include "SNetResolver.h7"
  ClassDef(SNetResolver, 0);
}; // endclass SNetResolver

#endif
