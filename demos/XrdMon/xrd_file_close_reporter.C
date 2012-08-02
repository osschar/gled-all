// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdFileCloseReporter that sends file-access reports as UDP messages.

class XrdFileCloseReporter;
class XrdFileCloseReporterGratia;

XrdFileCloseReporter       *c_frep        = 0;
XrdFileCloseReporterGratia *c_frep_gratia = 0;

void xrd_file_close_reporter()
{
  // This is a base-class ... it just prints name of the closed file to ZLog.
  // ASSIGN_ADD_GLASS(c_frep, XrdFileCloseReporter, g_queen, "XrdFileCloseReporter", 0);
  // c_frep->SetLog(c_log);
  // c_suck->AddFileCloseReporter(c_frep);
  // c_frep->StartReporter();

  // Sends text-like report as a UDP packet ... to be passed on to Gratia.
  ASSIGN_ADD_GLASS(c_frep_gratia, XrdFileCloseReporterGratia, g_queen, "XrdFileCloseReporterGratia", 0);
  c_frep_gratia->SetLog(c_log);

  // Set addresshost and port to which UDP file-access report should be sent:
  // c_frep_gratia->SetUdpHost("localhost");
  // c_frep_gratia->SetUdpPort(4242);

  c_suck->AddFileCloseReporter(c_frep_gratia);
  c_frep_gratia->StartReporter();
}
