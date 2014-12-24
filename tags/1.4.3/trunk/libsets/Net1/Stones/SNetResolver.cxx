// $Id: Stone_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SNetResolver.h"
#include "Stones/SUdpPacket.h"

#include <netdb.h>
#include <arpa/inet.h>

// SNetResolver

//______________________________________________________________________________
//
//

ClassImp(SNetResolver);

//==============================================================================

void SNetResolver::_init()
{}

SNetResolver::SNetResolver() :
  m_hostname_re    ("^([^\\.]+)\\.(.*)$", "o"),
  m_nodomain_re    ("^[^\\.]+$", "o"),
  m_ip4_numeric_re ("^(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)$", "o"),
  m_ip4_in_name_re ("^(.*\\d+\\.\\d+\\.\\d+\\.\\d+.*?)\\.([^\\.]+(?:\\.[^\\.]+)+)$", "o")
{
  _init();
}

SNetResolver::~SNetResolver()
{}

//==============================================================================

Bool_t SNetResolver::is_numeric(const TString& hostname)
{
  return m_ip4_numeric_re.Match(hostname) == 5;
}

Bool_t SNetResolver::was_local()
{
  UChar_t a = m_ip4_numeric_re[1].Atoi();
  UChar_t b = m_ip4_numeric_re[2].Atoi();

  return (a == 10) ||
    (a == 172 && b >= 16 && b <= 31) ||
    (a == 192 && b == 168);
}

Bool_t SNetResolver::is_fqhn(const TString& hostname)
{
  return m_hostname_re.Match(hostname) == 3;
}

Bool_t SNetResolver::is_nodomain(const TString& hostname)
{
  return m_nodomain_re.Match(hostname) == 1;
}

//==============================================================================

Bool_t SNetResolver::resolve_fqhn
(
  SUdpPacket *p,
  TString    &fqhn
)
{
  // Tries to determine fqhn of address that sent the UDP packet p.
  // If name lookup fails, it is translated into numeric form and
  // the state bool is_numeric is set to true;

  sockaddr_in  sa4;
  sockaddr_in6 sa6;
  sockaddr    *sa = 0;
  void        *sa_in = 0;
  socklen_t    sl;
  int          af;

  reset_status_bits();

  if (p->mAddrLen == 4)
  {
    af = sa4.sin_family = AF_INET;
    memcpy(&sa4.sin_addr.s_addr, p->mAddr, p->mAddrLen);
    sa = (sockaddr*) &sa4;
    sa_in = & sa4.sin_addr;
    sl = sizeof(sa4);
  }
  else
  {
    af = sa6.sin6_family = AF_INET6;
    memcpy(sa6.sin6_addr.s6_addr, p->mAddr, p->mAddrLen);
    sa = (sockaddr*) &sa6;
    sa_in = & sa6.sin6_addr;
    sl = sizeof(sa6);
  }

  Char_t buf[256];
  if (getnameinfo((sockaddr*) sa, sl, buf, sizeof(buf), 0, 0, NI_NAMEREQD) != 0)
  {
    if ( ! inet_ntop(af, sa_in, buf, sl))  return false;
    f_numeric = true;
  }
  fqhn = buf;
  fqhn.ToLower();

  return true;
}

Bool_t SNetResolver::split_non_numeric_fqhn_to_host_domain
(
  const TString &fqhn,
  TString &host, TString &domain
)
{
  // Returns false if none of the regexps match.
  // Sets f_local_domain flag if it's a short hostname (no dots in fqhn).

  if (m_ip4_in_name_re.Match(fqhn))
  {
    host   = m_ip4_in_name_re[1];
    domain = m_ip4_in_name_re[2];
  }
  else if (m_hostname_re.Match(fqhn) == 3)
  {
    host   = m_hostname_re[1];
    domain = m_hostname_re[2];
  }
  else if (m_nodomain_re.Match(fqhn) == 1)
  {
    host   = m_nodomain_re[0];
    domain = "local";
    f_local_domain = true;
  }
  else
  {
    return false;
  }

  return true;
}

Bool_t SNetResolver::split_fqhn_to_host_domain_no_lookup
(
  const TString &fqhn,
  TString &host, TString &domain
)
{
  // Returns false if none of the regexps match.
  // If fqhn is numeric, domain is either "unknown" or "local".
  // Status bits are set accordingly.

  reset_status_bits();

  if (is_numeric(fqhn))
  {
    f_numeric = true;
    host = fqhn;
    if (was_local()) { domain = "local";   f_local_domain   = true; }
    else             { domain = "unknown"; f_unknown_domain = true; }
  }
  // Eventually, check if numeric IPv6:
  // else if (is_numeric6(fqhn))
  else
  {
    return split_non_numeric_fqhn_to_host_domain(fqhn, host, domain);
  }

  return true;
}

Bool_t SNetResolver::split_fqhn_to_host_domain_with_lookup
(
  const TString& fqhn_in,
  TString &host, TString &domain
)
{
  // Returns false if things can not be figured out.

  TString fqhn = fqhn_in;

  reset_status_bits();

  if (is_numeric(fqhn))
  {
    if (was_local())
    {
      host    = fqhn;
      domain  = "local";
      f_numeric = true;
      f_local_domain = true;
      return true;
    }
    else
    {
      sockaddr_in sa4;
      sa4.sin_family = AF_INET;
      sa4.sin_port   = 0;
      inet_pton(AF_INET, fqhn, &sa4.sin_addr);

      Char_t buf[256];
      if (getnameinfo((sockaddr*) &sa4, sizeof(sa4), buf, sizeof(buf), 0, 0, NI_NAMEREQD) == 0)
      {
        fqhn = buf;
      }
      else
      {
        host   =  fqhn;
        domain = "unknown";
        f_numeric = true;
        f_unknown_domain = true;
        return true;
      }
    }
  }
  // Eventually, check if numeric IPv6 ... else if (is_numeric6(fqhn))

  return split_non_numeric_fqhn_to_host_domain(fqhn, host, domain);
}
