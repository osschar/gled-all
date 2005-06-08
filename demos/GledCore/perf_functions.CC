// $Header$

// Functions to perform performance measurements.

void post_mir(ZMIR* mir)
{
  g_saturn->PostMIR(mir);
  delete mir;
}

void run_eventor(Eventor* e)
{
  post_mir( e->S_Reset() );
  post_mir( e->S_Start() );
  do {
    gSystem->Sleep(500);
  } while(e->GetRunning() || e->GetTotalTime()==0);
}

TH1F* measure_nps(Eventor* e, PerfMeterOperator* pmo,
		  const Text_t* name, Int_t N, Int_t M)
{
  Double_t delta = 0.05;

  Double_t nps;
  TH1F* histo;

  post_mir( e->S_SetBeatsToDo(N) );
  for(int i=0; i<M; ++i) {
    run_eventor(e);
    nps = N/e->GetTotalTime();
    if(i==0) {
      histo = new TH1F(name, GForm("%s Test", name),
		       100, nps*(1-delta), nps*(1+delta));
    }
    histo->Fill(nps);
  }

  return histo;
}

void measure_st(Eventor* e, PerfMeterOperator* pmo)
{
  Float_t means[7], rmss[20]; int vi=0;
  Int_t N = (Int_t)(500000 * sun->GetSaturnInfo()->GetCPU_Freq() / 2133.0);
  Int_t M = 20;
  TH1F* h;
  ZMIR* m;

  post_mir( pmo->S_SetTest(PerfMeterOperator::TT_Void) );
  h = measure_nps(e, pmo, "Void", 4*N, M);
  printf("Void: %f %d\n", h->GetMean(), h->GetRMS());
  vi = 0; means[vi] = h->GetMean(); rmss[vi] = h->GetRMS();

  post_mir( pmo->S_SetTest(PerfMeterOperator::TT_Null) );
  h = measure_nps(e, pmo, "Null", N, M);
  printf("Null: %f %d\n", h->GetMean(), h->GetRMS());
  vi = 2; means[vi] = h->GetMean(); rmss[vi] = h->GetRMS();

  post_mir( e->S_SetMultix(true) );
  h = measure_nps(e, pmo, "Null Multix", 4*N, M);
  printf("Null Multix: %f %d\n", h->GetMean(), h->GetRMS());
  post_mir( e->S_SetMultix(false) );
  vi = 1; means[vi] = h->GetMean(); rmss[vi] = h->GetRMS();

  post_mir( pmo->S_SetTest(PerfMeterOperator::TT_IncCount) );
  h = measure_nps(e, pmo, "IncCount", N, M);
  printf("IncCount: %f %d\n", h->GetMean(), h->GetRMS());
  vi = 5; means[vi] = h->GetMean(); rmss[vi] = h->GetRMS();

  post_mir( pmo->S_SetTest(PerfMeterOperator::TT_IncCount) );

  post_mir( e->S_SetMultix(true) );
  h = measure_nps(e, pmo, "IncCount Multix", 4*N, M);
  printf("IncCount Multix: %f %d\n", h->GetMean(), h->GetRMS());
  post_mir( e->S_SetMultix(false) );
  vi = 4; means[vi] = h->GetMean(); rmss[vi] = h->GetRMS();

  // Start client
  printf("Starting client ...\n");
  SaturnInfo* client;
  client_executor->Start();
  do {
    gSystem->Sleep(500);
    client = (SaturnInfo*) sun->GetSaturnInfo()->GetMoons()->First();
  } while(client==0);
  gSystem->Sleep(2000);
  printf("Client up: %s\n", client->GetName());

  post_mir( e->S_SetHost(client) );
  post_mir( pmo->S_SetBeamHost(sun->GetSaturnInfo()) );
  post_mir( pmo->S_SetUseBeams(true) );

  post_mir( pmo->S_SetTest(PerfMeterOperator::TT_Null) );
  h = measure_nps(e, pmo, "Null Beamed", N, M);
  printf("Null Beamed: %f %d\n", h->GetMean(), h->GetRMS());
  vi = 3; means[vi] = h->GetMean(); rmss[vi] = h->GetRMS();

  post_mir( pmo->S_SetTest(PerfMeterOperator::TT_IncCount) );
  h = measure_nps(e, pmo, "IncCount Beamed", N, M);
  printf("IncCount Beamed: %f %d\n", h->GetMean(), h->GetRMS());
  vi = 6; means[vi] = h->GetMean(); rmss[vi] = h->GetRMS();

  post_mir( pmo->S_SetUseBeams(false) );
  post_mir( pmo->S_SetBeamHost(0) );
  post_mir( e->S_SetHost(0) );

  // Stop client ...
  client_executor->Stop();

  for(int i=0; i<7; ++i) {
    printf("\\valerr{%.1f}{%.1f} & ", means[i]/1000, rmss[i]/1000);
  }
  printf("\n");
}
