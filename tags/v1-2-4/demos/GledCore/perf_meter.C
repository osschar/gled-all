// $Header$

// Simple set-up for performance measurements.
// Spawns Eye automatically.
//
// vars: ZQueen* scenes
{
  if(Gled::theOne->GetSaturn() == 0) {
    gROOT->Macro("sun.C");
  }

  Scene* pm_scene  = new Scene("Performance Meters");
  scenes->CheckIn(pm_scene);
  scenes->Add(pm_scene);


  PipeEventor* client_executor = new PipeEventor("Client Executor");
  const char*  client_host     = "f9pc02";

  client_executor->SetCommand
    (
     GForm("ssh %s ~/bin/gled_run.sh gr1 binenv.sh %s 9061 > %s 2>&1",
	   client_host,
	   sun->GetSaturnInfo()->GetHostName(),
	   client_host)
     );
  client_executor->SetWaitTimeMS(1000);
  scenes->CheckIn(client_executor); pm_scene->Add(client_executor);

  PerfMeterTarget* pmt = new PerfMeterTarget("PMTarget");
  scenes->CheckIn(pmt); pm_scene->Add(pmt);

  // Eventor and PerfMeterOperator

  Eventor *dynamo = new Eventor;
  dynamo->SetBeatsToDo(100000);
  dynamo->SetStampInterval(100000);
  dynamo->SetInterBeatMS(0);
  scenes->CheckIn(dynamo); pm_scene->Add(dynamo);

  PerfMeterOperator* pmo = new PerfMeterOperator("PMOperator");
  pmo->SetTarget(pmt);
  scenes->CheckIn(pmo); dynamo->Add(pmo);

  printf("Loading perf_functions.CC\n");
  gROOT->ProcessLine(".L perf_functions.CC");

  gROOT->ProcessLine(".L eye.C");
  eye("Foofoo eye", "Boo shell", 0);

}
