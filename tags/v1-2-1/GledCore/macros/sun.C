// Generick initialization of top level script variables for Sun.
// Creates ZQueen("Scenes").
// Can be called from other scripts, as a command-line option or from
// TRint prompt.

{
  // This is needed if main was NOT run with -r flag
  if(Gled::theOne->GetSaturn() == 0) {
    Gled::theOne->SpawnSun();
  }

  Saturn* sun = Gled::theOne->GetSaturn();
  if(sun == 0) {
    printf("Sun is not spawned ...\n");
    return;
  }

  ZSunQueen* sun_queen  = sun->GetSunQueen();
  ZQueen*    fire_queen = sun->GetFireQueen();

  ZQueen* scenes = new ZQueen(16*1024, "Scenes", "Goddess of Ver");
  Gled::theOne->GetSaturn()->GetSunKing()->Enthrone(scenes);
  scenes->SetMandatory(true);
}
