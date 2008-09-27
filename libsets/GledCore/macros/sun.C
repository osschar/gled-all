// $Header$

// Generick initialization of top level script variables for Sun.
// Can be called from other scripts, as a command-line option or from
// TRint prompt.

void sun()
{
  if(Gled::theOne->GetSaturn() == 0) {
    Gled::theOne->SpawnSun();
    if(Gled::theOne->GetSaturn()->GetSaturnInfo()->GetUseAuth())
      gROOT->Macro("std_auth.C");
  }

  Gled::AssertMacro("gled_globals.C");
}
