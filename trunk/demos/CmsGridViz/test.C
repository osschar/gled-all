#include <glass_defines.h>

// Run at xrootd.t2:
// nc -u -l 9933 | nc -u greed.physics.ucsd.edu 9930

void test()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("CmsGridViz");

  g_queen->SetName("XrdMonitorQueen");
  CREATE_ADD_GLASS(suck, XrdMonSucker, g_queen, "XrdMonSucker", 0);
  suck->SetKeepSorted(true);

  //============================================================================
  // Spawn GUI

  Gled::LoadMacro("eye.C");
  eye(false);

  g_nest->Add(g_queen);
  g_nest->SetMaxChildExp(3); // This doesn't work ... only on creation.
  g_nest->SetWName(50);

  suck->StartSucker();
}
