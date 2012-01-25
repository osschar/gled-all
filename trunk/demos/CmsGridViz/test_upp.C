#include <glass_defines.h>

ZLog                 *c_log  = 0;

void test_upp()
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("CmsGridViz");

  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l = l->AssertPath("XrdStuff", "ZList");
    l->Swallow(new ZGlass("XrdUser", "XrdUser(FromHost,FromDomain)"));
  }

  g_queen->SetName("UDP Queen");

  CREATE_ADD_GLASS(upp, UdpPacketProcessor, g_queen, "UdpPacketProcessor", "X");
  upp->SetSuckPort(9929);

  CREATE_ADD_GLASS(cli, UdpPacketTcpClient, g_queen, "UdpPacketTcpClient", "Y");
  cli->SetHost("desire.physics.ucsd.edu");

  ASSIGN_ADD_GLASS(c_log, ZLog, g_queen, "XrdMonSucker Log", 0);
  // c_log->SetFileName("/var/log/xrootd/udp-packet-proc.log");
  c_log->SetFileName("udp-packet-proc.log");
  c_log->SetLevel(ZLog::L_Info);
  c_log->StartLogging();

  upp->SetLog(c_log);

  //============================================================================
  // Spawn GUI

  Gled::LoadMacro("eye.C");
  eye(false);

  g_nest->Add(g_queen);
  // g_nest->SetMaxChildExp(3); // This only works on creation.
  g_nest->SetWName(50);

  g_saturn->ShootMIR( upp->S_StartAllServices() );

  g_saturn->ShootMIR( cli->S_ConnectAndListenLoop() );
}
