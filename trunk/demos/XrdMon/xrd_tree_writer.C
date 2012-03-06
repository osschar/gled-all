// Add-on macro (expects xrd_suck_tcp/udp.C or xrd_udp_forwarder to be
// executed before).
// Runs UdpPacketTreeWriter that stores all packets into a tree.

class UdpPacketTreeWriter;
UdpPacketTreeWriter  *c_treew = 0;

void xrd_tree_writer()
{
  ASSIGN_ADD_GLASS(c_treew, UdpPacketTreeWriter, g_queen, "XrdTreeWriter", 0);
  c_treew->SetLog(c_log);
  c_treew->SetSource(c_udp_packet_source);

  g_saturn->ShootMIR( c_treew->S_Start() );
}
