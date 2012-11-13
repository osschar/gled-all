// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdFileCloseReporterAmq that sends file-access reports via ActiveMQ.

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

  // *** DO NOT COMMIT ***
  c_frep_amq->SetAmqUser("xrdpop");
  c_frep_amq->SetAmqPswd("montthree");

  // Set topic where the message is to be sent:
  // c_frep_amq->SetAmqTopic("xrdpop.uscms_test_popularity");

  // Number of messages to keep in memory while connection to AMQ is down.
  // After that older messages get dropped.
  // c_frep_amq->SetAmqMaxMsgQueueLen(10000);

  // Wait time after connection failure. On each new attempt the wait time will
  // be doubled.
  // c_frep_amq->SetAmqReconnectWaitSec(1);

  // Maximum time to wait between two connection attempts.
  // c_frep_amq->SetAmqReconnectWaitSecMax(300);


  c_suck->AddFileCloseReporter(c_frep_amq);

  c_frep_amq->StartReporter();
}
