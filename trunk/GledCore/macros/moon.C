// Initialization of top level script variables for Moon.
// Also spawns an eye.
// Preferably run as: gled -m <master> -r moon.C

{
  // a moon MUST be started with -r flag
  Saturn* sun = Gled::theOne->GetSaturn();
  if(sun == 0) {
    cerr <<
     "Saturn is not spawned ... type  Gled::theOne->SpawnSaturn() or consider\n"
     "using the -r command-line option.\n";
    return;
  }

  ZSunQueen* sun_queen  = sun->GetSunQueen();
  ZQueen*    fire_queen = sun->GetFireQueen();

  ZQueen* scenes = sun->GetSunKing()->GetElementByName("Scenes");

  gROOT->Macro("eye.C");	// spawn an eye with a pupil on first scene
}
