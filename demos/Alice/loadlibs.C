void loadlibs ()
{
  // Macro which loads the libraries needed for simulation and reconstruction
  // Possible usage: In a Root session (no AliRoot) one does
  // root [0] .x loadlibs.C
  // root [1] gAlice = new AliRun("gAlice","test")
  // root [2] AliSimulation sim
  // root [3] sim.Run()
  // root [4] AliReconstruction rec
  // root [5] rec.Run()

  gSystem->Load("libPhysics");
  gSystem->Load("libMinuit");

  // Uncomment the following line for macosx
  // Waiting for a better solution
  //  gSystem->Load("libg2c_sh");
  gSystem->Load("libmicrocern");
  gSystem->Load("liblhapdf");
  gSystem->Load("libpythia6");

  gSystem->Load("libEG");
  gSystem->Load("libGeom");
  gSystem->Load("libVMC");

  gSystem->Load("libEGPythia6");

  gSystem->Load("libNet");
  gSystem->Load("libTree");

  gSystem->Load("libSTEERBase");
  gSystem->Load("libESD");
  gSystem->Load("libCDB");
  gSystem->Load("libRAWDatabase");
  gSystem->Load("libRAWDatarec");
  gSystem->Load("libSTEER");
  gSystem->Load("libRAWDatasim");
  gSystem->Load("libEVGEN");
  gSystem->Load("libFASTSIM");
  gSystem->Load("libAliPythia6");

  gSystem->Load("libhijing");
  gSystem->Load("libTHijing");// AliGenHijingEventHeader needed by libZDCsim.so

  gSystem->Load("libSTRUCT");
  //gSystem->Load("libPHOSbase");
  //gSystem->Load("libPHOSsim");
  //gSystem->Load("libPHOSrec");
  gSystem->Load("libMUONmapping");
  gSystem->Load("libMUONgeometry");
  gSystem->Load("libMUONcalib");
  gSystem->Load("libMUONraw");
  gSystem->Load("libMUONtrigger");
  gSystem->Load("libMUONbase");
  gSystem->Load("libMUONsim");
  gSystem->Load("libMUONrec");
  gSystem->Load("libMUONevaluation");
  gSystem->Load("libFMDbase");
  gSystem->Load("libFMDsim");
  gSystem->Load("libFMDrec");
  gSystem->Load("libPMDbase");
  gSystem->Load("libPMDsim");
  gSystem->Load("libPMDrec");
  gSystem->Load("libHMPIDbase");
  gSystem->Load("libHMPIDsim");
  gSystem->Load("libHMPIDrec");
  gSystem->Load("libT0base");
  gSystem->Load("libT0sim");
  gSystem->Load("libT0rec");
  gSystem->Load("libZDCbase");
  gSystem->Load("libZDCsim");
  gSystem->Load("libZDCrec");
  gSystem->Load("libACORDEbase");
  gSystem->Load("libACORDEsim");
  gSystem->Load("libACORDErec");
  gSystem->Load("libVZERObase");
  gSystem->Load("libVZEROsim");
  gSystem->Load("libVZEROrec");
  //gSystem->Load("libEMCALbase");
  //gSystem->Load("libEMCALsim");
  //gSystem->Load("libEMCALrec");
  //gSystem->Load("libEMCALjet");

  // The following lines have to be commented on Darwin
  // for the moment due to cross dependencies
  gSystem->Load("libTPCbase");
  gSystem->Load("libTPCrec");
  gSystem->Load("libTPCsim");
  gSystem->Load("libTPCfast");
  gSystem->Load("libITSbase");
  gSystem->Load("libITSsim");
  gSystem->Load("libITSrec");
  //gSystem->Load("libTRDbase");
  //gSystem->Load("libTRDsim");
  //gSystem->Load("libTRDrec");
  //gSystem->Load("libTRDfast");
  gSystem->Load("libTOFbase");
  gSystem->Load("libTOFsim");
  gSystem->Load("libTOFrec");

  //gSystem->Load("libHLTbase");
  //gSystem->Load("libAliHLTITS");
  //gSystem->Load("libAliHLTSrc");
  //gSystem->Load("libAliHLTMisc");
  //gSystem->Load("libAliHLTComp");
  //gSystem->Load("libThread");
  //gSystem->Load("libAliHLTHough");
}
