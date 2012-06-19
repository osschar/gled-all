// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdFileCloseReporterTree that stores file-access-reports into a tree.

class XrdFileCloseReporterTree;
XrdFileCloseReporterTree  *c_tree_far = 0;

void xrd_far_tree_writer()
{
  ASSIGN_ADD_GLASS(c_tree_far, XrdFileCloseReporterTree, g_queen, "XrdFarTreeWriter", 0);
  c_tree_far->SetLog(c_log);
  c_suck->AddFileCloseReporter(c_tree_far);
  c_tree_far->StartReporter();
}
