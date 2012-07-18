// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdFileCloseReporterAmq that sends a file-access report via ActiveMQ.

class XrdFileCloseReporter;
class XrdFileCloseReporterAmq;

XrdFileCloseReporter    *c_frep     = 0;
XrdFileCloseReporterAmq *c_frep_amq = 0;

void xrd_file_close_reporter_amq()
{
  // Sends JSON report via ActiveMQ
  ASSIGN_ADD_GLASS(c_frep_amq, XrdFileCloseReporterAmq, g_queen, "XrdFileCloseReporterAmq", 0);
  c_frep_amq->SetLog(c_log);

  // Set addresshost and port where ActiveMQ server is running
  // c_frep_amq->SetAmqHost("gridmsg007.cern.ch");
  // c_frep_amq->SetAmqPort(6163);

  // Set username and password to be used:
  // c_frep_amq->SetAmqUser("xrdpop");
  // c_frep_amq->SetAmqPswd("xyzz");

  // Set topic where the message is to be sent:
  // c_frep_amq->SetAmqTopic("xrdpop.uscms_test_popularity");

  c_suck->AddFileCloseReporter(c_frep_amq);

  c_frep_amq->StartReporter();
}
