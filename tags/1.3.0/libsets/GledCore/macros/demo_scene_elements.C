// $Id$

// Complex scene elements used in demos.

void demo_scene_elements()
{}

/**************************************************************************/

void _dse_init_orto_base(ZList* cbs, ZList* cis, Float_t z_size,
			 Float_t dist, Int_t coord, Int_t up_axis,
			 const Text_t* bname, const Text_t* iname)
{
  TVector3 p_vec, x_vec, y_vec, z_vec;

  CREATE_ADD_GLASS(base, Sphere, cbs, bname, 0);
  int up_coord = up_axis;
  if(up_coord == coord) { --up_coord; if(up_coord < 1) up_coord = 3; }

  p_vec[coord - 1]    = dist;
  x_vec[coord - 1]    = -TMath::Sign(1.0, dist);
  z_vec[up_coord - 1] = 1;
  y_vec = z_vec.Cross(x_vec);

  ZTrans t;
  t.SetBaseVec(1, x_vec); t.SetBaseVec(2, y_vec);
  t.SetBaseVec(3, z_vec); t.SetBaseVec(4, p_vec);
  base->SetTrans(t);

  base->SetRadius(0.002);

  CREATE_ADD_GLASS(info, CameraInfo, cis, iname, 0);
  info->SetCameraBase(base);
  info->SetProjMode(CameraInfo::P_Orthographic);
  info->SetZSize(z_size);
}

void dse_make_camera_bases(ZList* cont, ZNode* up_ref, Int_t up_axis,
			   Float_t orto_dist,
			   Float_t px, Float_t py, Float_t pz)
{
  // Creates camera bases and camera infos for orthographic viewing.
  // The last parameters define location of "default perspective view
  // (look at origin).
  //
  // Supported values for up_axis: 2 ~ y; 3 ~ z.

  CREATE_ADD_GLASS(mark, ZNode, cont, "Markers", "");
  CREATE_ADD_GLASS(cis, ZNode, mark, "CameraInfos", "");
  CREATE_ADD_GLASS(cbs, ZNode, mark, "CameraBases", "");

  CREATE_ATT_GLASS(nms, ZRlNodeMarkup, cbs, SetRnrMod, "NamesOn", 0);
  nms->SetRnrTiles(false); nms->SetRnrFrames(false);
  nms->SetTextCol(0.62, 1, 0.64);
  nms->SetTileCol(0,0,0,0);

  {
    CREATE_ADD_GLASS(base, Sphere, cbs, "", "");
    TVector3 p_vec, x_vec, y_vec, z_vec;
    p_vec.SetXYZ(px, py, pz);
    x_vec = -p_vec.Unit();
    z_vec[up_axis - 1] = 1;
    y_vec = z_vec.Cross(x_vec);
    ZTrans t;
    t.SetBaseVec(1, x_vec); t.SetBaseVec(2, y_vec);
    t.SetBaseVec(3, z_vec); t.SetBaseVec(4, p_vec);
    t.OrtoNorm3();
    base->SetTrans(t);
    base->SetRadius(0.002);

    CREATE_ADD_GLASS(info, CameraInfo, cis, "Default", "");
    info->SetCameraBase(base);
    info->SetUpReference(up_ref);
    info->SetUpRefAxis(up_axis);
    info->SetProjMode(CameraInfo::P_Perspective);
    info->SetZFov(80);
  }

  _dse_init_orto_base(cbs, cis, 12,  orto_dist, 3, up_axis, "Z+", "Orto Z+");
  _dse_init_orto_base(cbs, cis, 12, -orto_dist, 3, up_axis, "Z-", "Orto Z-");

  _dse_init_orto_base(cbs, cis, 12,  orto_dist, 2, up_axis, "Y+", "Orto Y+");
  _dse_init_orto_base(cbs, cis, 12, -orto_dist, 2, up_axis, "Y-", "Orto Y-");

  _dse_init_orto_base(cbs, cis, 12,  orto_dist, 1, up_axis, "X+", "Orto X+");
  _dse_init_orto_base(cbs, cis, 12, -orto_dist, 1, up_axis, "X-", "Orto X-");
}
