// $Header$

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

void arc()
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }

  Scene* arcs  = new Scene("Arcs");
  scenes->CheckIn(arcs);
  scenes->Add(arcs);

  Box* pi_box = new Box("First box", "Base of pillar");
  pi_box->SetA(1); pi_box->SetB(1); pi_box->SetC(3);
  scenes->CheckIn(pi_box);
  // not added to the scene!

  boxer(4, pi_box, scenes, 22.5*TMath::Pi()/180, 0.98, 0.8);

  Box* base_box = new Box("Base box");
  base_box->RotateLF(1, 2, TMath::Pi()/4);
  base_box->SetABC(12, 12, 1);
  base_box->SetColor(0.9, 0.8, 1);
  scenes->CheckIn(base_box);
  arcs->Add(base_box);

  for(int i=0; i<4; ++i) {
    ZNodeLink* l = new ZNodeLink(GForm("Link%d", i+1));
    l->RotateLF(1,2, i*TMath::Pi()/2);
    l->MoveLF(1, -4.6);
    l->MoveLF(3, 1.6);
    scenes->CheckIn(l);
    base_box->Add(l);
    l->SetGlass(pi_box);
  }

  Box* b = new Box("Top box");
  b->SetABC(1.1, 1.1, 1);
  b->MoveLF(3, 6.9);
  b->SetColor(0.6, 0.2, 0.2);
  scenes->CheckIn(b);
  base_box->Add(b);

  Lamp* lamp = new Lamp("Lamp");
  lamp->SetScale(1);
  lamp->SetDiffuse(0.8, 0.8, 0.8);
  lamp->Set3Pos(5, -5, 10); lamp->RotateLF(1,2, TMath::Pi());
  scenes->CheckIn(lamp); arcs->Add(lamp);
  arcs->GetGlobLamps()->Add(lamp);


  // Spawn GUI
  gROOT->ProcessLine(".x eye.C");
}
