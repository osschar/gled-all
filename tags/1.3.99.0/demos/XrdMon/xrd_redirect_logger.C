// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Sets up logging of redirection messages.

ZLog *c_redir_log  = 0;

void xrd_redirect_logger()
{
  ASSIGN_ADD_GLASS(c_redir_log, ZLog, g_queen, "XrdMonSucker Redirect Log", 0);
  // c_redir_log->SetFileName("/var/log/gled/xrdmon-redirect.log");
  c_redir_log->SetFileName("xrdmon-redirect.log");

  // Verbose print-out, prints raw messages.
  // c_redir_log->SetLevel(ZLog::L_Info);

  c_redir_log->StartLogging();

  c_suck->SetRedirectLog(c_redir_log);
}
