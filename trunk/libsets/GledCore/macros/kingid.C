{
SaturnInfo* sat = new SaturnInfo;
sat->SetMasterName("localhost");
sat->SetMasterPort(9061);
sat->QueryFFID();
sat->Dump();
}
