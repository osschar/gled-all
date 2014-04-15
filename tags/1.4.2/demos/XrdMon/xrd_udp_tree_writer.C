// Add-on macro (expects xrd_suck_tcp/udp.C or xrd_udp_forwarder to be
// executed before).
// Runs UdpPacketTreeWriter that stores all packets into a tree.

class UdpPacketTreeWriter;
UdpPacketTreeWriter *c_tree_udp = 0;

void xrd_udp_tree_writer()
{
  ASSIGN_ADD_GLASS(c_tree_udp, UdpPacketTreeWriter, g_queen, "XrdUdpTreeWriter", 0);
  c_tree_udp->SetSource(c_udp_packet_source);
  c_tree_udp->SetLog(c_log);

  // c_tree_udp->SetAutoSaveEntries(100000);
  // c_tree_udp->SetAutoSaveMinutes(60);
  // c_tree_udp->SetRotateMinutes(24*60);
  // c_tree_udp->SetRotateAtMidnight(true);

  // c_tree_udp->SetFileIdxAlways(true);
  // c_tree_udp->SetFilePrefix("udp-tree-dump-");
  // c_tree_udp->SetTreeName("UdpPackets");
  c_tree_udp->SetTreeName("XrdUdp");

  g_gled->AfterSetup( c_tree_udp->S_Start() );
}
