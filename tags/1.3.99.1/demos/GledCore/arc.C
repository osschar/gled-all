// $Id$

#include <glass_defines.h>
#include <gl_defines.h>

void boxer(Int_t n, Box* p, ZQueen* m, Float_t phi, Float_t bf, Float_t cf)
{
  if(n<=0) return;
  Float_t cphi = TMath::Cos(phi), sphi = TMath::Sin(phi);
  Box* t = new Box();
  t->SetABC(cphi*p->GetA(), bf*p->GetB(), cf*p->GetC());
  Float_t foo = (t->GetC() - sphi*p->GetA())/2;
  t->MoveLF(1, foo*sphi);
  t->MoveLF(3, foo*cphi + p->GetC()/2);
  t->RotateLF(3,1,phi);
  m->CheckIn(t);
  p->Add(t);
  boxer(n-1, t, m, phi, bf, cf);
}

/**************************************************************************/

void arc(Int_t Npil = 5)
{
  Gled::AssertMacro("sun_demos.C");

  Scene* arcs  = new Scene("Arcs");
  g_queen->CheckIn(arcs);
  g_queen->Add(arcs);
  g_scene = arcs;

  Box* pi_box = new Box("First box", "Base of pillar");
  pi_box->SetA(1); pi_box->SetB(1); pi_box->SetC(3);
  g_queen->CheckIn(pi_box);
  // Not added to the scene directly but linked from a loop below.

  boxer(4, pi_box, g_queen, 22.5*TMath::Pi()/180, 0.98, 0.8);


  ZNode* base_node = new ZNode("Base Node");
  g_queen->CheckIn(base_node);
  arcs->Add(base_node);

  Cylinder* bc = new Cylinder("Base Cylinder");
  bc->RotateLF(1, 2, TMath::Pi()/2);
  bc->SetHeight(1);
  bc->SetROutBase(12); bc->SetROutTop(10);
  bc->SetColor(0.9, 0.8, 1);
  bc->SetLodPhi(Npil);
  g_queen->CheckIn(bc);
  arcs->Add(bc);

  for(int i=0; i<Npil; ++i) {
    ZNodeLink* l = new ZNodeLink(GForm("Link%d", i+1));
    l->RotateLF(1,2, i*TMath::TwoPi()/Npil);
    l->MoveLF(1, -4.6);
    l->MoveLF(3, 1.6);
    g_queen->CheckIn(l);
    base_node->Add(l);
    l->SetLens(pi_box);
  }

  Cylinder* tc = new Cylinder("Top Stone");
  tc->MoveLF(3, 6.9);
  tc->RotateLF(1, 2, -TMath::Pi()/2);
  tc->SetHeight(1.2);
  tc->SetRInBase(0.1); tc->SetRInTop(0.3);
  tc->SetROutBase(1);  tc->SetROutTop(1.2);
  tc->SetColor(0.45, 0.45, 1);
  tc->SetLodH(3);
  tc->SetLodPhi(Npil);
  g_queen->CheckIn(tc);
  base_node->Add(tc);

  Box* b = new Box("Table");
  b->SetABC(1.1, 1.1, 1);
  b->MoveLF(3, 1);
  b->SetColor(0.6, 0.1, 0.3);
  g_queen->CheckIn(b);
  base_node->Add(b);

  Lamp* lamp = new Lamp("Lamp");
  lamp->SetScale(1);
  lamp->SetDiffuse(0.8, 0.8, 0.8);
  lamp->SetPos(5, -5, 10); lamp->RotateLF(1,2, TMath::Pi());
  g_queen->CheckIn(lamp); arcs->Add(lamp);
  arcs->GetGlobLamps()->Add(lamp);


  // Spawn GUI
  Gled::Macro("eye.C");
  setup_pupil_up_reference();
}
