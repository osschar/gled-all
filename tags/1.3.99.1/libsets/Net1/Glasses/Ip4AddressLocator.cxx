// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Ip4AddressLocator.h"
#include "Ip4AddressLocator.c7"

#include "TPRegexp.h"

// Ip4AddressLocator

//______________________________________________________________________________
//
//

ClassImp(Ip4AddressLocator);

//==============================================================================

void Ip4AddressLocator::_init()
{}

Ip4AddressLocator::Ip4AddressLocator(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

Ip4AddressLocator::~Ip4AddressLocator()
{}

//==============================================================================

void Ip4AddressLocator::LoadFromCsvFile(const TString& fname)
{
  // Loads entries from a CSV text file as provided by HostIp project.

  static const Exc_t _eh("Ip4AddressLocator::LoadFromCsvFile ");

  struct ParseEngine
  {
    typedef hash_map<TString, Int_t>  hStr2Int_t;
    typedef hStr2Int_t::iterator      hStr2Int_i;

    hStr2Int_t loc_map;
    hStr2Int_i loc_it;

    Ip4AddressLocator &L;

    TPMERegexp ip_city_re;

    Int_t      loc_count, same_count;
    UInt_t     ip,  prev_ip;
    TString    loc, prev_loc;

    ParseEngine(Ip4AddressLocator& l) :
      L(l),
      ip_city_re("^(\\d+),(.*)$", "o"),
      loc_count(0), same_count(0),
      prev_ip(-1)
    {}

    void commit_entry()
    {
      // if (same_count > 127)
      // {
      //   printf("Committing %8x   %3d   %s\n", prev_ip, same_count, prev_loc.Data());
      // }

      L.mIpVec.push_back(prev_ip + same_count);
      L.mLocIdxVec.push_back(loc_it->second);
      same_count = 0;
    }

    void parse_line(const TString& line)
    {
      if (ip_city_re.Match(line) != 3)
      {
        throw _eh + GForm("Parse error on: '%s'.", line.Data());
      }

      ip  = static_cast<UInt_t>(ip_city_re[1].Atoll());
      loc = ip_city_re[2];
    }

    void process_first_line()
    {
      L.mLocInfoVec.push_back(IpAddressLocation(loc));
      loc_it = loc_map.insert(make_pair(loc, loc_count++)).first;

      prev_ip  = ip;
      prev_loc = loc;

    }

    void process_line()
    {
      // printf("  %08x %08x %d %d  '%s '%s\n", ip, prev_ip, ip == prev_ip + 0x100, loc == prev_loc, loc.Data(), prev_loc.Data());
      if (ip == prev_ip + 0x100 && loc == prev_loc)
      {
        if (++same_count == 0xff)
        {
          commit_entry();
        }
      }
      else
      {
        commit_entry();

        loc_it = loc_map.find(loc);
        if (loc_it == loc_map.end())
        {
          L.mLocInfoVec.push_back(IpAddressLocation(loc));
          loc_it = loc_map.insert(make_pair(loc, loc_count++)).first;
        }
      }

      prev_ip  = ip;
      prev_loc = loc;
    }
  };

  FILE *fp = fopen(fname, "r");

  ParseEngine pe(*this);

  Int_t   count = 0;
  TString line;

  line.Gets(fp);
  pe.parse_line(line);
  pe.process_first_line();

  while (line.Gets(fp))
  {
    pe.parse_line(line);
    pe.process_line();
    ++count;
  }
  pe.commit_entry();

  fclose(fp);

  printf("Done parsing: count=%d, ip_vec_size=%zd, loc_vec_size=%zd\n", count, mIpVec.size(), mLocInfoVec.size());
  printf("Fraction of original entries in compressed version: %f\n", ((float)mIpVec.size()) / count);
  printf("Number of locations: pe.loc_count=%d\n", pe.loc_count);

  // Printout first hundred vector enries
  if (true)
  {
    for (Int_t i = 0; i < 100; ++i)
    {
      UInt_t ip = mIpVec[i];

      UInt_t num = ip & 0xff;
      UInt_t max = (ip & 0xffffff00);
      UInt_t min = max - (num << 8);
  
      const IpAddressLocation& loc = mLocInfoVec[mLocIdxVec[i]];
      printf("%3d %10x %10x %10x %3u  --  %u %s (%s) at (%f, %f)\n", i, ip, min, max, num,
             mLocIdxVec[i], loc.GetCity(), loc.GetState(), loc.GetLatitude(), loc.GetLongitude());
    }
  }
}

void Ip4AddressLocator::QueryHostIp(UInt_t ip4)
{
  printf("Hello, going in for %08x\n", ip4);

  ip4 &= 0xffffff00;

  vUInt_i i = lower_bound(mIpVec.begin(), mIpVec.end(), ip4);

  if (i == mIpVec.end())
  {
    printf("Not in ipvec ...\n");
    return;
  }

  UInt_t num = *i & 0xff;
  UInt_t max = (*i & 0xffffff00);
  UInt_t min = max - (num << 8);

  if (ip4 >= min && ip4 <= max)
    printf("Yay, match! %s\n", mLocInfoVec[mLocIdxVec[i-mIpVec.begin()]].GetCity());
  else
    printf("Yebo, no luck\n");
}
