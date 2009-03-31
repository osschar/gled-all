// $Id$

// Common global variables.

// Saturn related information.

Saturn*     g_saturn     = 0;

ZKing*      g_sun_king   = 0;
ZKing*      g_king       = 0;
ZFireKing*  g_fire_king  = 0;

ZSunQueen*  g_sun_queen  = 0;
ZFireQueen* g_fire_queen = 0;

// Current containers (used by eye.C (a GUI spawner) and demo scripts.
// Can be modified by the user.

ZQueen*     g_queen       = 0;
Scene*      g_scene       = 0;

void gled_globals()
{
  g_saturn = Gled::theOne->GetSaturn();
  if(g_saturn == 0) {
    throw(Exc_t("gled_globals: Sun is not spawned.\n"));
  }

  g_sun_king   = g_saturn->GetSunKing();
  g_king       = g_saturn->GetKing();
  g_fire_king  = g_saturn->GetFireKing();

  g_sun_queen  = g_saturn->GetSunQueen();
  g_fire_queen = g_saturn->GetFireQueen();
}
