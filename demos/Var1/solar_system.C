#include <glass_defines.h>
#include <gl_defines.h>

class SolarSystem;

SolarSystem* ss = 0;

void solar_system()
{
  gRandom = new TRandom3(0); // Seed 0 means seed with TUUID.

  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Var1");

  g_queen->SetName("SSQueen");
  g_queen->SetTitle("Demo of a SolarSystem");

  ASSIGN_ADD_GLASS(g_scene, Scene, g_queen, "Solar System Scene", "Testing functionality of Var1::SolarSystem glass.");

  ASSIGN_ADD_GLASS(ss, SolarSystem, g_scene, "SolarSystem", "");
  ss->SetColor(0.8, 0.8, 0.8, 0.8);
  ss->GetODECrawler()->SetStoreDx(10);

  ss->MakeStar();
  for (Int_t i = 0; i < 30; ++i)
  {
    ss->MakePlanetoid();
  }

  Gled::Macro("eye.C");
  setup_pupil_up_reference();
  g_pupil->SetNearClip(1);
  g_pupil->SetFarClip(5000);
  g_pupil->SetMoveOM(-1);
}
