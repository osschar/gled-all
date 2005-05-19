// simple EHS test
//
// vars: ZQueen* scenes
// libs: EHS

#include <glass_defines.h>
#include <memory>

void ehs_test() {
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->LoadMacro("sun.C");
    sun(false);
  }
  Gled::theOne->AssertLibSet("EHS");

  ehsq = new ZQueen(256*1024, "EHS Queen", "");
  Gled::theOne->GetSaturn()->GetSunKing()->Enthrone(ehsq);

  CREATE_ADD_GLASS(ehs, EHSWrap, ehsq, "EHS Wrap", 0);
  ehs->SetPort(4242);

  // Spawn GUI
  gROOT->LoadMacro("eye.C");
  eye("EHS Monitor", "Shell", 0);

  sun->ShootMIR( ehs->S_StartServer() );
}
