// simple EHS test
//
// vars: ZQueen* scenes
// libs: EHS

#include <glass_defines.h>

void ehs_test() {
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("EHS");

  ehsq = new ZQueen(256*1024, "EHS Queen", "");
  g_saturn->GetSunKing()->Enthrone(ehsq);

  CREATE_ADD_GLASS(ehs, EHSWrap, ehsq, "EHS Wrap", 0);
  ehs->SetPort(4242);

  // Spawn GUI
  Gled::LoadMacro("eye.C");
  eye(false);
  g_nest->ImportKings();

  g_saturn->ShootMIR( ehs->S_StartServer() );
}
