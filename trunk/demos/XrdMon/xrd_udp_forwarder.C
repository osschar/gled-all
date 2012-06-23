#include <glass_defines.h>

class UdpPacketSource;
class UdpPacketListener;
class UdpPacketTcpServer;

ZLog               *c_log      = 0;
UdpPacketListener  *c_listener = 0;
UdpPacketTcpServer *c_server   = 0;

UdpPacketSource    *c_udp_packet_source = 0;

void xrd_udp_forwarder()
{
  Gled::AssertMacro("sun.C");
  Gled::theOne->AssertLibSet("XrdMon");

  g_queen = new ZQueen(256*1024, "Queen of Udp to Tcp Conversion");
  g_sun_king->Enthrone(g_queen);
  g_queen->SetMandatory(true);

  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("XrdStuff", "ZList");
    l->Swallow(new ZGlass("XrdUser", "XrdUser(FromHost,FromDomain)"));
  }

  ASSIGN_ADD_GLASS(c_log, ZLog, g_queen, "XrdMonSucker Log", 0);
  // c_log->SetFileName("/var/log/gled/xrdmon-udp-forwarder.log");
  c_log->SetFileName("xrdmon-udp-forwarder.log");
  c_log->SetLevel(ZLog::L_Info);
  c_log->StartLogging();

  ASSIGN_ADD_GLASS(c_listener, UdpPacketListener, g_queen, "UdpPacketListener", "");
  c_listener->SetLog(c_log);
  // Listens for UDP traffic on port 9930.
  // c_listener->SetSuckPort(9930);
  // Now running on desire, second slot:
  c_listener->SetSuckPort(9929);
  c_udp_packet_source = c_listener;

  ASSIGN_ADD_GLASS(c_server, UdpPacketTcpServer, g_queen, "UdpPacketTcpServer", "");
  c_server->SetLog(c_log);  
  c_server->SetSource(c_listener);
  // Listens for incoming TCP on 9940
  // c_server->SetServerPort(9940);


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
  g_gled->AfterSetup( c_listener->S_StartAllServices() );
  g_gled->AfterSetup( c_server  ->S_StartAllServices() );
}
