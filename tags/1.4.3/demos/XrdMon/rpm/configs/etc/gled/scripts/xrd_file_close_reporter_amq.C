// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdFileCloseReporterAmq that sends file-access reports via ActiveMQ.

#include <cstdlib>

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
  // c_frep_amq->SetAmqHost("dashb-mb.cern.ch");
  // c_frep_amq->SetAmqPort(6163);

  // Set username and password to be used:
  // c_frep_amq->SetAmqUser("xrdpop");
  // c_frep_amq->SetAmqPswd("xyzz");

  // Set topic where the message is to be sent:
  // c_frep_amq->SetAmqTopic("xrdpop.uscms_test_popularity");

  // activemq-cpp apparently double-deletes some objects.
  // Set this flag to true to try to work around this by simply leaking all
  // AMQ objects in amq_disconnect().
  // c_frep_amq->SetLeakAmqObjs(false);

  // Number of messages to keep in memory while connection to AMQ is down.
  // After that older messages get dropped.
  // c_frep_amq->SetAmqMaxMsgQueueLen(10000);

  // Wait time after connection failure. On each new attempt the wait time will
  // be doubled.
  // c_frep_amq->SetAmqReconnectWaitSec(1);

  // Maximum time to wait between two connection attempts.
  // c_frep_amq->SetAmqReconnectWaitSecMax(300);

  // Reconnect to AMQ server periodically. The default value of 0 means
  // that the client will stay connected until there is an error.
  // c_frep_amq->SetAmqAutoReconnectSec(0);

  c_frep_amq->SetAmqHost( getenv("MSG_HOST") );
  c_frep_amq->SetAmqPort( atoi(getenv("MSG_PORT")) );

  c_frep_amq->SetAmqUser( getenv("MSG_USER") );
  c_frep_amq->SetAmqPswd( getenv("MSG_PASSWD") );

  c_frep_amq->SetAmqTopic( getenv("MSG_TOPIC") );


  c_suck->AddFileCloseReporter(c_frep_amq);

  g_gled->AfterSetup( c_frep_amq->S_StartReporter() );
}
