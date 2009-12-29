void mac_hack()
{   
  gSystem->Load("libGledCore_Pupils");
  gROOT->ProcessLine(".L gled_view_globals.C");
  gROOT->ProcessLine(".L eye.C");
}