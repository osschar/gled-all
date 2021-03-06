// Listens directly for xrdmon UDP traffic.
// Processes this info with XrdMonSucker + runs file-close reporter
// and embedded http server showing opened files.
//
// Consider running UDP -> TCP server, maybe one at every big site.
// See xrd_udp_forwarder.C and xrd_suck_tcp.C.

#include <cstdlib>
#include <glass_defines.h>

class UdpPacketSource;
class UdpPacketListener;
class XrdMonSucker;

ZLog              *c_log  = 0;
UdpPacketListener *c_listener = 0;
XrdMonSucker      *c_suck = 0;

UdpPacketSource   *c_udp_packet_source = 0;

void xrd_suck_udp()
{
  Gled::AssertMacro("sun.C");
  Gled::theOne->AssertLibSet("XrdMon");

  g_queen = new ZQueen(1024*1024, "XrdMonitorQueen");
  g_sun_king->Enthrone(g_queen);
  g_queen->SetMandatory(true);

  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("XrdStuff", "ZList");
    l->Swallow(new ZGlass("XrdUser", "XrdUser(FromHost,FromDomain)"));
  }

  ASSIGN_ADD_GLASS(c_log, ZLog, g_queen, "XrdMonSucker Log", 0);
  // c_log->SetFileName("/var/log/gled/xrdmon-suck-udp.log");
  c_log->SetFileName( getenv("LISTENER_LOGFILE") );

  // For verbose logging (every packet received).
  // c_log->SetLevel(ZLog::L_Info);

  ASSIGN_ADD_GLASS(c_listener, UdpPacketListener, g_queen, "UdpPacketListener", "");
  c_listener->SetLog(c_log);
  // Listens for UDP traffic on port 9930.
  // c_listener->SetSuckPort(9930);
  c_listener->SetSuckPort( atoi(getenv("LISTEN_PORT")) );
  c_udp_packet_source = c_listener;

  ASSIGN_ADD_GLASS(c_suck, XrdMonSucker, g_queen, "XrdMonSucker", 0);
  c_suck->SetKeepSorted(true);
  c_suck->SetLog(c_log);
  c_suck->SetSource(c_listener);
  // Storing of individual io requests into XrdFile.
  // NOTE: To store them into FAR tree it should also be enable on the
  // XrdFileCloseReporterTree object.
  // c_suck->SetStoreIoInfo(false);


  //============================================================================

  // Spawn GUI
  if (Gled::theOne->HasGUILibs())
  {
    Gled::LoadMacro("eye.C");
    eye(false);
    g_nest->Add(g_queen);
    g_nest->SetWName(50);
  }

  // Start threads
  c_log->StartLogging();
  c_suck->StartSucker();

  g_gled->AfterSetup( c_listener->S_StartAllServices() );
}
