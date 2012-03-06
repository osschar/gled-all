// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdFileCloseReporter that sends a file-close report as a UDP message.

class XrdFileCloseReporter;
XrdFileCloseReporter *c_frep = 0;

voidxrd_file_close_reporter()
{
  ASSIGN_ADD_GLASS(c_frep, XrdFileCloseReporter, g_queen, "XrdFileCloseReporter", 0);
  c_frep->SetLog(c_log);
  // c_frep->SetUdpHost("localhost");
  // c_frep->SetUdpPort(4242);

  c_suck->SetFCReporter(c_frep);

  c_frep->StartReporter();
}
