// $Header$
// Generick initialization of top level script variables for Sun.
// Creates ZQueen("Scenes").
// Can be called from other scripts, as a command-line option or from
// TRint prompt.

Saturn*     sun        = 0;
Saturn*     saturn     = 0;

ZSunQueen*  sun_queen  = 0;
ZFireQueen* fire_queen = 0;
ZQueen*     scenes     = 0;

void sun(bool create_scenes_queen=true)
{
  // This is needed if main was NOT run with -r flag
  if(Gled::theOne->GetSaturn() == 0) {
    Gled::theOne->SpawnSun();
  }

  sun = Gled::theOne->GetSaturn();
  if(sun == 0) {
    printf("Sun is not spawned ...\n");
    return;
  }
  saturn = sun;

  sun_queen  = sun->GetSunQueen();
  fire_queen = sun->GetFireQueen();

  if(sun->GetSaturnInfo()->GetUseAuth())
    gROOT->Macro("std_auth.C");

  if(create_scenes_queen) {
    scenes = new ZQueen(256*1024, "Scenes", "Goddess of Ver");
    Gled::theOne->GetSaturn()->GetSunKing()->Enthrone(scenes);
    scenes->SetMandatory(true);
  }

}
