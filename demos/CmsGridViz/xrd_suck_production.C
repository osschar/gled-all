#include <glass_defines.h>

void xrd_suck_production()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("CmsGridViz");

  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("XrdStuff", "ZList");
    l->Swallow(new ZGlass("XrdUser", "XrdUser(FromHost,FromDomain)"));
  }

  g_queen->SetName("XrdMonitorQueen");
  CREATE_ADD_GLASS(suck, XrdMonSucker, g_queen, "XrdMonSucker", 0);
  suck->SetKeepSorted(true);

  CREATE_ADD_GLASS(fcloserep, XrdFileCloseReporter, g_queen, "XrdFileCloseReporter", 0);
  fcloserep->SetUdpHost("brian-test.unl.edu");
  fcloserep->SetUdpPort(3334);
  suck->SetFCReporter(fcloserep);


  //============================================================================
  // Spawn GUI

  Gled::LoadMacro("eye.C");
  eye(false);

  g_nest->Add(g_queen);
  // g_nest->SetMaxChildExp(3); // This only works on creation.
  g_nest->SetWName(50);

  // Regexps for setting full-trace-print flag for new user sessions.
  suck->SetTraceDN("Matevz Tadel");
  suck->SetTraceHost("uaf-");
  suck->SetTraceDomain("ucsd.edu");

  suck->StartSucker();

  fcloserep->StartReporter();
}
