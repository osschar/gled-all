// $Header$

// Simple scene with a Moonraker (shooting from earth in earth/moon system)
// properly attached to/with ODE_Crawler (ODE integrator).
//
// Moon is set really close to earth (6 earth radii).
// Set parameter of the system in Moonraker
// Set parameters of ODE in ODE_Crawler. Use ODE_Crawler->Start() to
// compute trajectory. No check for collision with earth/moon is done.
// Change Moonraker->T variable to animate shell movement.
//
// vars: ZQueen* scenes
// libs: Numerica, Geom1
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }
  Gled::theOne->AssertLibSet("Numerica");
  Gled::theOne->AssertLibSet("Geom1");

  Scene* moon_scene  = new Scene("Moonraker Scene");
  scenes->CheckIn(moon_scene);
  scenes->Add(moon_scene);

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(20);
  base_plane->SetColor(0.6, 0.6, 0.6);
  scenes->CheckIn(base_plane);
  moon_scene->Add(base_plane);

  ODECrawler* ode_c = new ODECrawler();
  scenes->CheckIn(ode_c);
  moon_scene->Add(ode_c);
  ode_c->SetAcc(1e-12);
  ode_c->SetStoreMax(2000);

  Moonraker* moon = new Moonraker();
  scenes->CheckIn(moon);
  moon_scene->Add(moon);
  moon->SetDMoon(6);
  moon->SetT1(0.25);
  moon->SetTheta(3.5); moon->SetPhi(1.7);
  moon->SetV0(0.9148);

  // Set links.
  ode_c->SetODEMaster(moon);
  moon->SetODECrawler(ode_c);

  ode_c->Start();


  // Spawn GUI
  gROOT->Macro("eye.C");
  if(pupil) {
    // Comment to disable fixing of camera 'up' direction to 'z' axis.
    pupil->SetUpReference(moon_scene);
    pupil->SetUpRefAxis(3);
  }
}
