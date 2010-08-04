#include <glass_defines.h>
#include <gl_defines.h>

const Text_t* bush_layout = "ZGlass(Name,Title[22])";

void tree(int mode = 0)
{
  Gled::AssertMacro("sun_demos.C");
  Gled::theOne->AssertLibSet("GledCore");
  Gled::theOne->AssertLibSet("Geom1");
  Gled::theOne->AssertLibSet("Tmp1");
  
  Scene* images  = new Scene("Images");
  g_queen->CheckIn(images);
  g_queen->Add(images);
  g_scene = images;

  //______________________________________________________________________________
  if (mode == 0)
  {
    MonopodialTree* tree = new MonopodialTree("MonopodialTree", "Simple MonopodialTree");    
    g_queen->CheckIn(tree);
    images->Add(tree);
    
    CREATE_ATT_GLASS(rules, ZVector, tree, SetRules, "Rules", 0);
   
    AddRule(tree, rules, "A", "!F[&B]/A");
    AddRule(tree, rules, "B", "!F[-$C]C");
    AddRule(tree, rules, "C", "!F[+$B]B");    
    tree->SetStart("A");   
    tree->SetLevel(10);
    tree->RotateLF(1, 3, TMath::Pi()*0.5);
  }
  else if ( mode == 1)
  {
    SympodialTree* tree = new SympodialTree("SympodilaTree", "Sypodila");
    
    g_queen->CheckIn(tree);
    images->Add(tree);
    
    
    CREATE_ATT_GLASS(rules, ZVector, tree, SetRules, "Rules", 0);
    
    AddRule(tree, rules, "A", "!F[&B]/[&B]");
    AddRule(tree, rules, "B", "!F[+$B][-$B]");
    tree->SetStart("A");   
    tree->SetLevel(10);
    tree->SetStartWidth(0.03);
    tree->RotateLF(1, 3, TMath::Pi()*0.5);
    tree->SetTrunkAngle(5);
    tree->SetLateralAngle(65);
  }
  else
  {
    TernaryTree* tree = new TernaryTree("TernaryTree", "Ternary");
    g_queen->CheckIn(tree);
    images->Add(tree);
    CREATE_ATT_GLASS(rules, ZVector, tree, SetRules, "Rules", 0);
    
    AddRule(tree, rules, "A", "!F[&FA]/[&FA]/[&FA]");
    tree->SetStart("!F/A");   
    tree->SetLevel(6);
    tree->SetStartWidth(0.001);
    tree->RotateLF(1, 3, TMath::Pi()*0.5);
    tree->SetTrunkAngle(19);
    tree->SetStartWidth(0.01);
    tree->SetStartLength(1);
    tree->SetTrunkContraction(1.109);
    tree->SetSusceptibility(0.22);
  }
  
  //______________________________________________________________________________  
  
   
  Rect* base_plane = new Rect("BasePlane");
  base_plane->SetUnitSquare(2);
  g_queen->CheckIn(base_plane);
  images->Add(base_plane);
  base_plane->SetColor(0.7, 0.7, 0.7);

   //______________________________________________________________________________

  // Spawn GUI
  Gled::Macro("eye.C");  
  g_pupil->SetMoveOM(-3);
  g_pupil->EmitCameraHomeRay();
  g_pupil->SetCHSize(0);
  
  {
    ZList* l = g_fire_queen;
    l = l->AssertPath(NestInfo::sLayoutPath, "ZNameMap");
    l->Swallow(new ZGlass("Production",
                          "ProductionRule(*)"));
    g_nest->SetLayout("ParametricSystem(Level[6],DumpInfo[4]):ProductionRule(Rule[20])");
    
    g_nest->SetLeafLayout(NestInfo::LL_Custom);
  }  
}

void 
AddRule(ZNode* cons, ZVector* holder,  const char* name, const char* title)
{
  CREATE_ADD_GLASS(rule1, ProductionRule, holder, name, title);
  rule1->SetConsumer(cons);
}

