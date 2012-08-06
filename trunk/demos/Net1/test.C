#include <glass_defines.h>

class Ip4AddressLocator;

Ip4AddressLocator *c_iploc = 0;

void test()
{
  Gled::AssertMacro("sun.C");
  Gled::theOne->AssertLibSet("Net1");

  g_queen = new ZQueen(256*1024, "Queen of Udp to Tcp Conversion");
  g_sun_king->Enthrone(g_queen);
  g_queen->SetMandatory(true);

  ASSIGN_ADD_GLASS(c_iploc, Ip4AddressLocator, g_queen, "Ip4AddressLocator", 0);

  //============================================================================

  // Spawn GUI
  if (Gled::theOne->HasGUILibs())
  {
    Gled::LoadMacro("eye.C");
    eye(false);
    g_nest->Add(g_queen);
    g_nest->SetWName(50);
  }

  // Load the csv db from hostip (sorted & fixed)
  // Available at ftp://ftp.gled.org/var/ip4locs-sorted.csv.gz:
  //   wget ftp://ftp.gled.org/var/ip4locs-sorted.csv.gz
  //   gunzip ip4locs-sorted.csv.gz
  g_gled->AfterSetup( c_iploc->S_LoadFromCsvFile("ip4locs-sorted.csv") );
}
