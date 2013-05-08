// Add-on macro (expects xrd_suck_tcp/udp.C to be executed before).
// Runs XrdFileCloseReporterTree that stores file-access-reports into a tree.

class XrdFileCloseReporterTree;
XrdFileCloseReporterTree *c_tree_far = 0;

void xrd_far_tree_writer()
{
  ASSIGN_ADD_GLASS(c_tree_far, XrdFileCloseReporterTree, g_queen, "XrdFarTreeWriter", 0);
  c_tree_far->SetLog(c_log);

  // c_tree_far->SetAutoSaveEntries(100000);
  // c_tree_far->SetAutoSaveMinutes(60);
  // c_tree_far->SetRotateMinutes(24*60);
  // c_tree_far->SetRotateAtMidnight(true);
  // c_tree_far->SetCondWaitSec(0);

  // Storing of individual io requests from XrdFile into the tree.
  // NOTE: Filling of IO-info structure in XrdFile should also be enabled on
  // the XrdMonSucker object!
  // c_tree_far->SetStoreIoInfo(false);
  // c_tree_far->SetFileIdxAlways(true);
  // c_tree_far->SetFilePrefix("xrd-file-access-report-");
  // c_tree_far->SetTreeName("XrdFar");

  g_gled->AfterSetup( c_suck->S_AddFileCloseReporter(c_tree_far) );
  g_gled->AfterSetup( c_tree_far->S_StartReporter() );
}
