// $Id$

// Generick initialization of top level script variables for Sun.
// Also creates global queen ZQueen("Scenes"), then used by demo-scripts
// ta add in scene objects.
// Can be called from other scripts, as a command-line option or from
// TRint prompt.

void sun_demos(Int_t queen_id_size=256*1024)
{
  Gled::Macro("sun.C");

  g_queen = new ZQueen(queen_id_size, "Scenes", "Goddess of Ver");
  g_sun_king->Enthrone(g_queen);
  g_queen->SetMandatory(true);
}
