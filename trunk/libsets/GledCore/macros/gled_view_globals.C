// $Header$

// Common global variables related to GUI and viewers.

EyeInfo*       g_eye   = 0;
ShellInfo*     g_shell = 0;
NestInfo*      g_nest  = 0;
PupilInfo*     g_pupil = 0;

void gled_view_globals()
{
  // Needed to determine if the script has been already loaded.

  GledNS::LoadSo("libGledCore_Pupils.so");
}
