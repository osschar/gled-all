// Initialization of top level script variables for Moon.
// Also spawns an eye.
// Preferably run as: gled -m <master> -r moon.C

Saturn*      moon      = 0;
Saturn*     saturn     = 0;

ZSunQueen*  sun_queen  = 0;
ZFireQueen* fire_queen = 0;
ZQueen*     scenes     = 0;

void moon()
{
  // a moon MUST be started with -r flag
  moon = Gled::theOne->GetSaturn();
  if(moon == 0) {
    cerr <<
     "Saturn is not spawned ... use the -r command-line option.\n";
    return;
  }
  saturn = moon;

  sun_queen  = saturn->GetSunQueen();
  fire_queen = saturn->GetFireQueen();

  scenes = (ZQueen*) saturn->GetSunKing()->GetElementByName("Scenes");

  gROOT->Macro("eye.C");	// spawn an eye with a pupil on first scene
}
