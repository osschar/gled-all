/**************************************************************************/
// Stripped down ZAliLoad view.
/**************************************************************************/

MetaViewInfo* make_zaliload_metagui()
{
  int Y = 0, W = 40, H = 27;

  CREATE_ADD_GLASS(mv, MetaViewInfo, g_fire_queen, "MetaGui for ZAliLoad", 0);
  mv->Size(W, H);

  int y = 0;

  // DataDir/Event handling
  CREATE_ADD_GLASS(ms1, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms1->Position(0, 0);
  // Intro
  CREATE_ADD_GLASS(w10, MetaWeedInfo, ms1, "<box>", "Data Source control:");
  w10->Resize(0, y, W, 1);
  w10->Color(0.85, 0.7, 0.7);
  w10->Align(true, -1, 0);
  w10->Box(MetaWeedInfo::BT_Engraved);
  y++;
  // members
  CREATE_ADD_GLASS(w11, MetaWeedInfo, ms1, "DataDir", 0);
  w11->Resize(6, y, W-12-6, 1);
  CREATE_ADD_GLASS(w12, MetaWeedInfo, ms1, "Event", 0);
  w12->Resize(W-6, y, 6, 1);
  y++;
  CREATE_ADD_GLASS(w17, MetaWeedInfo, ms1, "VSDFile", 0);
  w17->Resize(6, y, W-12-6, 1);
  CREATE_ADD_GLASS(w18, MetaWeedInfo, ms1, "LoadVSD", 0);
  w18->Resize(W-12, y, 12, 1);
  y++;
  CREATE_ADD_GLASS(w18, MetaWeedInfo, ms1, "ClearData", 0);
  w18->Resize(W-12, y, 12, 1);
  y++;
  CREATE_ADD_GLASS(w15, MetaWeedInfo, ms1, "<box>", "Current operation:");
  w15->Resize(0, y, W, 1);
  w15->Color(0.7, 0.7, 0.85);
  w15->Align(true, -1, 0);
  w15->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w14, MetaWeedInfo, ms1, "Operation", 0);
  w14->Resize(0, y, W, 1);
  y++;

  Y += y; y=0;

  // ### Data importers

  // Kinematics:
  CREATE_ADD_GLASS(ms2, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms2->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w20, MetaWeedInfo, ms2, "<box>", "Kinematics:");
  w20->Resize(0, y, W, 1);
  w20->Color(0.7, 0.85, 0.7);
  w20->Align(true, -1, 0);
  w20->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w21, MetaWeedInfo, ms2, "ParticleSelection", 0);
  w21->Resize(6, y, W-6, 1);
  w21->Label("selection: ");
  // w20->Align(false, 0, -1);
  y++;
  CREATE_ADD_GLASS(w22, MetaWeedInfo, ms2, "SelectParticles", 0);
  w22->Resize(W-12, y, 12, 1);
  w22->Label("Run selection ..");
  // w21->Align(false, 0, 1);
  y++;

  Y += y; y=0;

  // Hits:
  CREATE_ADD_GLASS(ms3, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms3->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w30, MetaWeedInfo, ms3, "<box>", "Hits:");
  w30->Resize(0, y, W, 1);
  w30->Color(0.7, 0.85, 0.7);
  w30->Align(true, -1, 0);
  w30->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w31, MetaWeedInfo, ms3, "HitSelection", 0);
  w31->Resize(6, y, W-6, 1);
  w31->Label("selection: ");
  y++;
  CREATE_ADD_GLASS(w32, MetaWeedInfo, ms3, "SelectHits", 0);
  w32->Resize(W-12, y, 12, 1);
  w32->Label("Run selection ..");
  y++;

  Y += y; y=0;

  // Clusters:
  CREATE_ADD_GLASS(ms5, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms5->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w50, MetaWeedInfo, ms5, "<box>", "Clusters:");
  w50->Resize(0, y, W, 1);
  w50->Color(0.7, 0.85, 0.7);
  w50->Align(true, -1, 0);
  w50->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w51, MetaWeedInfo, ms5, "ClusterSelection", 0);
  w51->Resize(6, y, W-6, 1);
  w51->Label("selection: ");
  y++;
  CREATE_ADD_GLASS(w52, MetaWeedInfo, ms5, "SelectClusters", 0);
  w52->Resize(W-12, y, 12, 1);
  w52->Label("Run selection ..");
  y++;

  Y += y; y=0;

  // RecTracks:
  CREATE_ADD_GLASS(ms6, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms6->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w60, MetaWeedInfo, ms6, "<box>", "RecTracks:");
  w60->Resize(0, y, W, 1);
  w60->Color(0.7, 0.85, 0.7);
  w60->Align(true, -1, 0);
  w60->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w61, MetaWeedInfo, ms6, "RecSelection", 0);
  w61->Resize(6, y, W-6, 1);
  w61->Label("selection: ");
  y++;
  CREATE_ADD_GLASS(w62, MetaWeedInfo, ms6, "SelectRecTracks", 0);
  w62->Resize(W-12, y, 12, 1);
  w62->Label("Run selection ..");
  ++y;

  Y += y; y=0;

  // V0:
  CREATE_ADD_GLASS(ms8, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms8->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w80, MetaWeedInfo, ms8, "<box>", "V0:");
  w80->Resize(0, y, W, 1);
  w80->Color(0.7, 0.85, 0.7);
  w80->Align(true, -1, 0);
  w80->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w81, MetaWeedInfo, ms8, "V0Selection", 0);
  w81->Resize(6, y, W-6, 1);
  w81->Label("selection: ");
  y++;
  CREATE_ADD_GLASS(w82, MetaWeedInfo, ms8, "SelectV0", 0);
  w82->Resize(W-12, y, 12, 1);
  w82->Label("Run selection ..");
  ++y;

  Y += y; y=0;

  // Kinks:
  CREATE_ADD_GLASS(ms8, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms8->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w80, MetaWeedInfo, ms8, "<box>", "Kinks:");
  w80->Resize(0, y, W, 1);
  w80->Color(0.7, 0.85, 0.7);
  w80->Align(true, -1, 0);
  w80->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w81, MetaWeedInfo, ms8, "KinkSelection", 0);
  w81->Resize(6, y, W-6, 1);
  w81->Label("selection: ");
  y++;
  CREATE_ADD_GLASS(w82, MetaWeedInfo, ms8, "SelectKinks", 0);
  w82->Resize(W-12, y, 12, 1);
  w82->Label("Run selection ..");
  ++y;

  Y += y; y=0;
  // GenInfo:
  CREATE_ADD_GLASS(ms7, MetaSubViewInfo, mv, "ZAliLoad", 0);
  ms7->Position(0, Y);
  // Intro
  CREATE_ADD_GLASS(w70, MetaWeedInfo, ms7, "<box>", "GenInfo:");
  w70->Resize(0, y, W, 1);
  w70->Color(0.7, 0.85, 0.7);
  w70->Align(true, -1, 0);
  w70->Box(MetaWeedInfo::BT_Engraved);
  y++;
  CREATE_ADD_GLASS(w71, MetaWeedInfo, ms7, "GISelection", 0);
  w71->Resize(6, y, W-6, 1);
  w71->Label("selection: ");
  y++;
  CREATE_ADD_GLASS(w72, MetaWeedInfo, ms7, "SelectGenInfo", 0);
  w72->Resize(W-12, y, 12, 1);
  w72->Label("Run selection ..");
  ++y;

  return mv;
}
