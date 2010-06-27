// $Id$

// Simple set-up for FPE signal tests.
// Spawns Eye automatically.
//
// vars: ZQueen* g_queen
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("Alice");

  Scene* pm_scene  = new Scene("FPE Scene");
  g_queen->CheckIn(pm_scene);
  g_queen->Add(pm_scene);

  Eventor     *e = 0;
  FpeOperator *o = 0;
  for (Int_t i = 1; i <= 4; ++i)
  {
    e = new Eventor(GForm("Eventor %d", i));
    e->SetBeatsToDo(-1);
    e->SetStampInterval(1000);
    e->SetInterBeatMS(1);
    e->SetTrapAll();
    g_queen->CheckIn(e); pm_scene->Add(e);

    o = new FpeOperator(GForm("FpeOperator %d", i));
    g_queen->CheckIn(o); e->Add(o);
  }

  // Control over full stack-trace dump. To disable:
  // gEnv->SetValue("Root.Stacktrace", 0);

  g_scene = pm_scene;
  Gled::LoadMacro("eye.C");
  eye(false);

  g_nest->SetLayout("ZGlass(Name):Eventor(Start,Stop,Running,Suspended):"
		    "FpeOperator(RaiseILL,RaiseBUS,RaiseSEGV,RaiseFPE)");
  g_nest->SetLeafLayout(NestInfo::LL_Custom);
}
