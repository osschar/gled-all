// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdEhs to serve a table of currently open files.

class XrdEhs;
XrdEhs *c_ehs  = 0;

void xrd_http_serve_open_files()
{
  ASSIGN_ADD_GLASS(c_ehs, XrdEhs, g_queen, "XrdEhs", 0);
  c_ehs->SetXrdSucker(c_suck);
  // c_ehs->SetPort(4242);

  g_gled->AfterSetup( c_ehs->S_StartServer() );
}
