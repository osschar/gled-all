// $Header$

// Top-level script that starts a Moon (client) connected to the Sun
// specified with -master argument of gled.
// Also spawns an eye with a pupil on the first scene of the queen "Scenes".
// Supposedly run as: gled -m <master> moon.C

void moon()
{
  if(Gled::theOne->GetSaturn() == 0) {
    Gled::theOne->SpawnSaturn();
  }

  Gled::AssertMacro("gled_globals.C");

  g_queen = (ZQueen*) g_sun_king->GetElementByName("Scenes");
  if(g_queen)
    g_scene = dynamic_cast<Scene*>(g_queen->FrontElement());

  Gled::Macro("eye.C"); // spawn an eye with a pupil on first scene
  setup_pupil_up_reference();
}
