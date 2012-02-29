// Listens directly for xrdmon UDP traffic.
// Processes this info with XrdMonSucker + runs file-close reporter
// and embedded http server showing opened files.
//
// Consider running UDP -> TCP server, maybe one at every big site.
// See xrd_udp_forwarder.C and xrd_suck_tcp.C.

#include <glass_defines.h>

class UdpPacketListener;
class XrdMonSucker;
class XrdFileCloseReporter;
class XrdEhs;

ZLog                 *c_log  = 0;
UdpPacketListener    *c_listener = 0;
XrdMonSucker         *c_suck = 0;
XrdFileCloseReporter *c_frep = 0;
XrdEhs               *c_ehs  = 0;

void xrd_suck_udp()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("XrdMon");

  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("XrdStuff", "ZList");
    l->Swallow(new ZGlass("XrdUser", "XrdUser(FromHost,FromDomain)"));
  }

  g_queen->SetName("XrdMonitorQueen");

  ASSIGN_ADD_GLASS(c_log, ZLog, g_queen, "XrdMonSucker Log", 0);
  // c_log->SetFileName("/var/log/xrootd/xrd-mon-sucker.log");
  c_log->SetFileName("xrd-mon-sucker.log");
  // For verbose logging (every packet received).
  // c_log->SetLevel(ZLog::L_Info);

  ASSIGN_ADD_GLASS(c_listener, UdpPacketListener, g_queen, "UdpPacketListener", "");
  c_listener->SetLog(c_log);
  // c_listener->SetSuckPort(9930);

  ASSIGN_ADD_GLASS(c_suck, XrdMonSucker, g_queen, "XrdMonSucker", 0);
  c_suck->SetKeepSorted(true);
  c_suck->SetLog(c_log);
  c_suck->SetSource(c_listener);

  ASSIGN_ADD_GLASS(c_frep, XrdFileCloseReporter, g_queen, "XrdFileCloseReporter", 0);
  c_frep->SetLog(c_log);
  // c_frep->SetUdpHost("localhost");
  // c_frep->SetUdpPort(4242);

  c_suck->SetFCReporter(c_frep);

  ASSIGN_ADD_GLASS(c_ehs, XrdEhs, g_queen, "XrdEhs", 0);
  c_ehs->SetXrdSucker(c_suck);
  // c_ehs->SetPort(4242);


  //============================================================================
  // Spawn GUI

  if (Gled::theOne->HasGUILibs())
  {
    Gled::LoadMacro("eye.C");
    eye(false);

    g_nest->Add(g_queen);
    g_nest->SetWName(50);
  }

  // Regexps for setting full-trace-print flag for new user sessions.
  c_suck->SetTraceDN("Matevz Tadel");
  c_suck->SetTraceHost("uaf-");
  c_suck->SetTraceDomain("ucsd.edu");

  c_log->StartLogging();

  c_suck->StartSucker();
  c_frep->StartReporter();

  g_saturn->ShootMIR( c_listener->S_StartAllServices() );
  g_saturn->ShootMIR( c_ehs->S_StartServer() );
}
