// $Header$

// Generick initialization of top level script variables for Sun.
// Also creates global queen ZQueen("Scenes"), then used by demo-scripts
// ta add in scene objects.
// Can be called from other scripts, as a command-line option or from
// TRint prompt.

void sun_demos()
{
  if(Gled::theOne->GetSaturn() == 0) {
    Gled::theOne->SpawnSun();
    if(Gled::theOne->GetSaturn()->GetSaturnInfo()->GetUseAuth())
      gROOT->Macro("std_auth.C");
  }

  Gled::AssertMacro("gled_globals.C");

  g_queen = new ZQueen(256*1024, "Scenes", "Goddess of Ver");
  g_sun_king->Enthrone(g_queen);
  g_queen->SetMandatory(true);
}
