#include <glass_defines.h>
#include <gl_defines.h>

class SolarSystem;
class CosmicBall;
class ODECrawler;
class TimeMaker;

SolarSystem  *ss       = 0;
CosmicBall   *stella   = 0;
ODECrawler   *ode      = 0;

Eventor      *eventor  = 0;
TimeMaker    *tmaker   = 0;
ScreenDumper *dumper   = 0;

void solar_system()
{
  gRandom = new TRandom3(0); // Seed 0 means seed with TUUID.

  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Var1");

  g_queen->SetName("SSQueen");
  g_queen->SetTitle("Demo of a SolarSystem");

  ASSIGN_ADD_GLASS(g_scene, Scene, g_queen, "Solar System Scene", "Testing functionality of Var1::SolarSystem glass.");

  CREATE_ADD_GLASS(lamp, Lamp, g_scene, "Lamp", "");
  lamp->SetPos(0, 0, 0);
  lamp->SetDiffuse(1, 1, 0.6);
  // lamp->SetDrawLamp(false);
  lamp->SetLampScale(1);
  // This crashes my laptop together with glDrawArrays of tringtvor ... badly.
  // Must be a weird ATI bug.
  g_scene->GetGlobLamps()->Add(lamp);

  ASSIGN_ADD_GLASS(ss, SolarSystem, g_scene, "SolarSystem", "");
  ss->SetColor(0.8, 0.8, 0.8, 0.8);
  ode = ss->GetODECrawler();

  ss->MakeStar();
  stella = dynamic_cast<CosmicBall*>(ss->GetBalls()->GetElementById(0));

  CREATE_ATT_GLASS(emission, ZGlMaterial, stella, SetRnrMod, "Emission", 0);
  emission->SetEmission(1, 1, 0.4);

  for (Int_t i = 0; i < 30; ++i)
  {
    ss->MakePlanetoid();
  }

  Gled::Macro("eye.C");
  setup_pupil_up_reference();
  g_pupil->SetNearClip(1);
  g_pupil->SetFarClip(5000);
  g_pupil->SetMoveOM(-1);
  g_pupil->SetWidth(1024);
  g_pupil->SetHeight(768);

  ode->SetStoreDx(10);
  ode->SetX1(0); ode->SetX2(100);
  ode->SetAcc(1e-8);
  ode->SetH1(1); ode->SetHmin(1e-3);
  ode->Crawl();

  ASSIGN_ADD_GLASS(eventor, Eventor, g_queen, "Eventor", 0);
  eventor->SetInterBeatMS(20);
  g_nest->Add(eventor);

  ASSIGN_ADD_GLASS(tmaker, TimeMaker, eventor, "TimeMaker", 0);

  tmaker->AddClient(ss);
  ss->SetTimeFac(10);

  ASSIGN_ADD_GLASS(dumper, ScreenDumper, eventor, "ScreenDumper", 0);
  dumper->SetWaitSignal(true);
  dumper->SetPupil(g_pupil);

  eventor->Start();
}
