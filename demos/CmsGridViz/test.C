#include <glass_defines.h>

class XrdMonSucker;
class XrdFileCloseReporter;
class XrdEhs;

ZLog                 *c_log  = 0;
XrdMonSucker         *c_suck = 0;
XrdFileCloseReporter *c_frep = 0;
XrdEhs               *c_ehs  = 0;

void test()
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

  ASSIGN_ADD_GLASS(c_suck, XrdMonSucker, g_queen, "XrdMonSucker", 0);
  c_suck->SetKeepSorted(true);

  ASSIGN_ADD_GLASS(c_frep, XrdFileCloseReporter, g_queen, "XrdFileCloseReporter", 0);
  // c_frep->SetUdpHost("desire.physics.ucsd.edu");
  // c_frep->SetUdpPort(7632);
  c_suck->SetFCReporter(c_frep);

  ASSIGN_ADD_GLASS(c_ehs, XrdEhs, g_queen, "XrdEhs", 0);
  c_ehs->SetXrdSucker(c_suck);
  c_ehs->SetPort(4242);

  ASSIGN_ADD_GLASS(c_log, ZLog, g_queen, "XrdMonSucker Log", 0);
  c_log->SetFileName("xrdmonsuck.log");
  c_log->SetLevel(ZLog::L_Info);
  c_suck->SetLog(c_log);


  //============================================================================
  // Spawn GUI

  Gled::LoadMacro("eye.C");
  eye(false);

  g_nest->Add(g_queen);
  // g_nest->SetMaxChildExp(3); // This only works on creation.
  g_nest->SetWName(50);

  // Regexps for setting full-trace-print flag for new user sessions.
  c_suck->SetTraceDN("Matevz Tadel");
  c_suck->SetTraceHost("uaf-");
  c_suck->SetTraceDomain("ucsd.edu");

  c_log->StartLogging();
  c_suck->StartSucker();
  c_frep->StartReporter();
  g_saturn->ShootMIR( c_ehs->S_StartServer() );
}
