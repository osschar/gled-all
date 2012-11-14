// Connects to server serving xrdmon UDP traffic via TCP.
// Processes this info with XrdMonSucker + runs file-close reporter
// and embedded http server showing opened files.
//
// Could really have several TCP (or, also, UDP) sources, e.g. one
// for each big site where we are running. Motivations:
// 1. UDP packets are the first to be dropped by routers during congestion.
//    I've seen this already, mostly from MIT and Wisonsin.
// 2. To compactify and compress the streams, sending payload once per
//    x seconds, or so.

#include <glass_defines.h>

class UdpPacketSource;
class UdpPacketTcpClient;
class XrdMonSucker;

ZLog               *c_log  = 0;
UdpPacketTcpClient *c_upc  = 0;
XrdMonSucker       *c_suck = 0;

UdpPacketSource    *c_udp_packet_source = 0;

void xrd_suck_tcp()
{
  Gled::AssertMacro("sun.C");
  Gled::theOne->AssertLibSet("XrdMon");

  g_queen = new ZQueen(256*1024, "XrdMonitorQueen");
  g_sun_king->Enthrone(g_queen);
  g_queen->SetMandatory(true);

  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("XrdStuff", "ZList");
    l->Swallow(new ZGlass("XrdUser", "XrdUser(FromHost,FromDomain)"));
  }

  ASSIGN_ADD_GLASS(c_log, ZLog, g_queen, "XrdMonSucker Log", 0);
  // c_log->SetFileName("/var/log/gled/xrdmon-suck-tcp.log");
  c_log->SetFileName("xrdmon-suck-tcp.log");
  // For verbose logging (every packet received).
  // c_log->SetLevel(ZLog::L_Info);

  ASSIGN_ADD_GLASS(c_upc, UdpPacketTcpClient, g_queen, "UdpPacketTcpClient", 0);
  c_upc->SetLog(c_log);
  c_upc->SetHost("xrootd.t2.ucsd.edu");
  // c_upc->SetHost("desire.physics.ucsd.edu");
  // c_upc->SetPort(9940);
  // Parameters controlling automatic reconnection.
  // c_upc->SetNRetry(120);
  // c_upc->SetRetryWaitSec(30);
  // c_upc->SetExitOnFailure(false);
  c_udp_packet_source = c_upc;

  ASSIGN_ADD_GLASS(c_suck, XrdMonSucker, g_queen, "XrdMonSucker", 0);
  c_suck->SetKeepSorted(true);
  c_suck->SetLog(c_log);
  c_suck->SetSource(c_upc);
  // Storing of individual io requests into XrdFile.
  // NOTE: To store them into FAR tree it should also be enable on the
  // XrdFileCloseReporterTree object.
  // c_suck->SetStoreIoInfo(false);

  // Debugging setup.
  // Regexps for setting full-trace-print flag for new user sessions.
  // c_suck->SetTraceDN("Matevz Tadel");
  // c_suck->SetTraceHost("uaf-");
  // c_suck->SetTraceDomain("ucsd.edu");


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
  c_log ->StartLogging();
  c_upc ->ConnectAndListenLoop();
  c_suck->StartSucker();
}
