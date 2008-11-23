// $Id$

// Simple scene with a Moonraker (shooting from earth in earth/moon system)
// properly attached to/with ODE_Crawler (ODE integrator).
//
// Moon is set really close to earth (6 earth radii).
// Set parameter of the system in Moonraker
// Set parameters of ODE in ODE_Crawler. Use ODE_Crawler->Start() to
// compute trajectory. No check for collision with earth/moon is done.
// Change Moonraker->T variable to animate shell movement.
//
// vars: ZQueen* g_queen
// libs: Numerica, Geom1
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Numerica");
  Gled::theOne->AssertLibSet("Geom1");

  Scene* moon_scene  = new Scene("Moonraker Scene");
  g_queen->CheckIn(moon_scene);
  g_queen->Add(moon_scene);
  g_scene = moon_scene;

  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(20);
  base_plane->SetColor(0.6, 0.6, 0.6);
  g_queen->CheckIn(base_plane);
  moon_scene->Add(base_plane);

  ODECrawler* ode_c = new ODECrawler();
  g_queen->CheckIn(ode_c);
  moon_scene->Add(ode_c);
  ode_c->SetAcc(1e-12);
  ode_c->SetStoreMax(2000);

  Moonraker* moon = new Moonraker();
  g_queen->CheckIn(moon);
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
  Gled::Macro("eye.C");
  setup_pupil_up_reference();
}
