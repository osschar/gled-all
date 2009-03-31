// monitor in SunQueen
//
// vars: ZSunQueen* sun_queen
// libs:
{

  Eventor* top_mon = new Eventor("Cluster Monitor");
  top_mon->SetStampInterval(1);
  top_mon->SetInterBeatMS(3*1000);
  top_mon->SetMultix(true);
  sun_queen->CheckIn(top_mon); sun_queen->Add(top_mon);

  GledMonitor* gm = new GledMonitor("Node status collector");
  gm->SetFillHistos(true);
  sun_queen->CheckIn(gm); top_mon->Add(gm);

  top_mon->Start();
}
