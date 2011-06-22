#include <glass_defines.h>

// Run at xrootd.t2:
// nc -u -l 9933 | nc -u greed.physics.ucsd.edu 9930

void test()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("CmsGridViz");

  CREATE_ADD_GLASS(suck, XrdMonSucker, g_queen, "XrdMonSucker", 0);

  //============================================================================
  // Spawn GUI

  Gled::LoadMacro("eye.C");
  eye(false);

  g_nest->Add(g_queen);

  suck->StartSucker();
}
