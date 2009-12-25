void mac_hack()
{   
   gSystem->Load("libGledCore_Pupils");
   gROOT->ProcessLine(".L gled_view_globals.C");
  	gROOT->ProcessLine(".L eye.C");
   
   Gled::theOne->LoadLibSet("Geom1");
   Gled::theOne->LoadLibSet("GTS");
   Gled::theOne->LoadLibSet("Audio1");
   Gled::theOne->LoadLibSet("Numerica");
   Gled::theOne->LoadLibSet("Var1");
}