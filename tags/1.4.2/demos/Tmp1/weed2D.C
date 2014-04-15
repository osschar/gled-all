// Simple scene with 4 example of 2D L-systems.
// Two examples are single rule and the rest two rules examples.

#include <glass_defines.h>
#include <gl_defines.h>

const Text_t* weed_layout = "ZGlass(Name):ProductionRule(Rule[22])";

void weed2D(Int_t mode = 0)
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("GledCore");
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("Tmp1");
  
  Scene* images  = new Scene("Images");
  g_queen->CheckIn(images);
  g_queen->Add(images);
  g_scene = images;
  
  CREATE_ADD_GLASS(base, Sphere, images, "cam base", "");
  base->SetPos(0, 0, 10);
  base->SetRotByDegrees(0, -90, 0);
  base->SetRadius(0.002);
  int level = 4;
  float dist  = 1.5;

  {
    Weed* weed = new Weed("Yellow(1)", "Single rule F[+F]F[-F]F");
    weed->SetLevel(level);
    g_queen->CheckIn(weed);
    images->Add(weed);
    CREATE_ATT_GLASS(rules, ZVector, weed, SetRules, "Rules", 0);
    CREATE_ADD_GLASS(rule1, ProductionRule, rules, "F", "F[+F]F[-F]F");
    
    weed->SetStart("F");
    weed->SetUseScale(kTRUE);
    weed->SetPos(-dist, -dist, 0);
    weed->SetScale(0.5);
    weed->SetAngle(25.7*TMath::DegToRad());
    weed->SetLineColor(1, 1, 0, 0);
  }
  {
    Weed* weed = new Weed("Red (1)", "Single rule F[+F]F[-F][F]");
    weed->SetLevel(level);
    g_queen->CheckIn(weed);
    images->Add(weed);
    CREATE_ATT_GLASS(rules, ZVector, weed, SetRules, "Rules", 0);
    AddRule(weed, rules, "F", "F[+F]F[-F][F]");
    weed->SetStart("F");
    weed->SetAngle(25.7*TMath::DegToRad());
    weed->SetPos(-dist, dist, 0.);
    weed->SetLineColor(1, 0, 0, 0);
  }
  {
    Weed* weed = new Weed("Magenta (2)", "Two rules X->F[+F]F[-F][F] F->FF");
    weed->SetLevel(level);
    g_queen->CheckIn(weed);
    images->Add(weed);
    CREATE_ATT_GLASS(rules, ZVector, weed, SetRules, "Rules", 0);
    AddRule(weed, rules, "X", "F[+X]F[-X]+X");
    AddRule(weed, rules,  "F", "FF");
    weed->SetStart("X");
    weed->SetAngle(25.7*TMath::DegToRad());      
    weed->SetPos(dist, -dist, 0.);
    weed->SetLineColor(1, 0, 1, 0);
  }
   
  {
    Weed* weed = new Weed("Cyan (2)", "Two rules X->F[+F]F[-F][F] F->FF");
    weed->SetLevel(level);
    g_queen->CheckIn(weed);
    images->Add(weed);
    CREATE_ATT_GLASS(rules, ZVector, weed, SetRules, "Rules", 0);
    AddRule(weed, rules, "X", "F-[[X]+X]+F[+FX]-X");    
    AddRule(weed, rules, "F", "FF");
    weed->SetStart("X");
    weed->SetAngle(25.7*TMath::DegToRad());      
    weed->SetPos(dist, dist, 0.);
    weed->SetLineColor(0, 1, 1, 0);
  }
  //______________________________________________________________________________
  
  // Spawn GUI
  Gled::Macro("eye.C");
  {
    ZList* l = g_fire_queen;
    g_nest->SetLayout("ZGlass(Name):Weed(Level[6], Angle[12]):ProductionRule(Rule[20])");

    g_nest->SetLeafLayout(NestInfo::LL_Custom);
  }
    
  setup_pupil_up_reference();
  g_pupil->SetCameraBase(base);
  g_pupil->SetProjMode(CameraInfo::P_Orthographic);
  cbase = base;   
  g_pupil->SetMoveOM(-4);
  g_pupil->SetUpRefAxis(1);
  
  g_pupil->EmitCameraHomeRay();
  g_pupil->SetCHSize(0.0);
}

void 
AddRule(ZNode* cons, ZVector* holder,  const char* name, const char* title)
{
  CREATE_ADD_GLASS(rule1, ProductionRule, holder, name, title);
  rule1->SetConsumer(cons);
}