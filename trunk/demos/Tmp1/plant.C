// Simple scene with four examples of 3D L-system.
// Has 3 modes:
//    (0) hilbert mode for debug 
//    (1) simple bush
//    (2) simple plant with flowers


#include <glass_defines.h>
#include <gl_defines.h>

const Text_t* bush_layout = "ZGlass(Name,Title[22])";

void plant(int mode = 2)
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("GledCore");
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("Tmp1");

  ZGlLightModel* lm = new ZGlLightModel("ZGlLightModel 1");
  g_queen->CheckIn(lm);
  lm->SetShadeModelOp(1);
  lm->SetFrontMode(GL_FILL);
  lm->SetBackMode(GL_FILL);
  lm->SetFaceCullOp(0);
  
  Scene* images  = new Scene("Images");
  g_queen->CheckIn(images);
  g_queen->Add(images);
  g_scene = images;
  
  CREATE_ADD_GLASS(base, Sphere, images, "cam base", "");
  base->SetPos(0, 0, 10);
  base->SetRotByDegrees(0, -90, 0);
  base->SetRadius(0.002);
  
  if ( mode == 0)
  {
    Plant* hilbert = new Plant("Hilbert", "Hibert 3D");
    g_queen->CheckIn(hilbert);
    images->Add(hilbert);
    CREATE_ATT_GLASS(rules, ZVector, hilbert, SetRules, "Rules", 0);
    AddRule(hilbert, rules, "A", "B-F+CFC+F-D&F^D-F+&&CFC+F+B//");
    AddRule(hilbert, rules, "B", "A&F^CFB^F^D^^-F-D^|F^B|FC^F^A//");
    AddRule(hilbert, rules, "C", "|D^|F^B-F+C^F^A&&FA&F^C+F+B^F^D//");
    AddRule(hilbert, rules, "D", "|CFB-F+B|FA&F^A&&FB-F+B|FC//");
    hilbert->SetStart("A");
    hilbert->SetAngle(TMath::PiOver2());    
    hilbert->SetLevel(2);
    hilbert->SetRnrMod(lm);
  }
  else if (mode == 1)
  { 
    Plant* bush = new Plant("Bush", "Simple Bush");
    g_queen->CheckIn(bush);
    images->Add(bush);
    CREATE_ATT_GLASS(rules, ZVector, bush, SetRules, "Rules", 0);
    AddRule(bush, rules, "A", "[&FL!A]/////@[&FL!A]///////@[&FL!A]");
    AddRule(bush, rules, "F", "S/////F");
    AddRule(bush, rules, "S", "FL");
    bush->SetStart("A");
    bush->SetAngle(22.5*TMath::DegToRad());    
    bush->SetLevel(4);
    bush->RotateLF(1, 3, TMath::Pi()*0.5);
    bush->SetLineColor(1, 0, 0);    
    bush->SetLeafColor(1, 0, 0);
  }
  else
  {
    Plant* plant = new Plant("Bush", "Simple Bush");
    g_queen->CheckIn(plant);
    images->Add(plant);
    CREATE_ATT_GLASS(rules, ZVector, plant, SetRules, "Rules", 0);
    AddRule(plant, rules, "p", "i+[p+C]--//[--L]i[++L]-[pC]++pC");
    AddRule(plant, rules, "i", "Fs[//&&L][//∧∧L]Fs");
    AddRule(plant, rules, "s", "sFs");    
    AddRule(plant, rules, "C", "[&&&FF/w////w////w////w////w////w]"); 
    AddRule(plant, rules, "w", "[^FO]");  
    plant->SetStart("p");
    plant->SetAngle(16*TMath::DegToRad());    
    plant->SetLevel(6);
    plant->RotateLF(1, 3, TMath::Pi()*0.5);
  }
  
  //______________________________________________________________________________
  
  // Spawn GUI
  Gled::Macro("eye.C");  
  g_pupil->SetMoveOM(-3);
  g_pupil->EmitCameraHomeRay();
  
  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l->Swallow(new ZGlass("Production",
                          "ZGlass(Name):ProductionRule(*)"));
    g_nest->SetLayout("ZGlass(Name):Weed(Level[6], Angle[12]):ProductionRule(Rule[20])");
    
    g_nest->SetLeafLayout(NestInfo::LL_Custom);
  }  
}

void 
AddRule(ZNode* cons, ZVector* holder,  const char* name, const char* title)
{
  CREATE_ADD_GLASS(rule1, ProductionRule, holder, name, title);
  rule1->SetConsumer(cons);
}

