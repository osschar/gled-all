// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdEhs to serve a table of currently open files.

class XrdEhs;
XrdEhs *c_ehs  = 0;

void xrd_http_serve_open_files()
{
  ASSIGN_ADD_GLASS(c_ehs, XrdEhs, g_queen, "XrdEhs", 0);
  c_ehs->SetXrdSucker(c_suck);

  // TCP port where browser connections are to be listened for:
  // c_ehs->SetPort(4242);

  // Paranoia mode (shorten file names and hash-up usernames):
  // c_ehs->SetParanoia(false);

  // Location of JS sortable table component:
  // c_ehs->SetWebTableJs("https://uaf-2.t2.ucsd.edu/~alja/gs_sortable.js");

  g_gled->AfterSetup( c_ehs->S_StartServer() );
}
