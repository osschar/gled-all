// eye.C
//
// assumes: ZQueen* scenes; ZQueen* fire_queen;

void eye(const Text_t* eye_name="Eye of Ra",
	 const Text_t* shell_name="Example Shell",
	 const Text_t* pupil_name="Example Pupil")
{
  ShellInfo* shell = new ShellInfo(shell_name);
  fire_queen->CheckIn(shell); fire_queen->Add(shell);
  shell->ImportKings();	  // Get all Kings as top level objects

  if(pupil_name) {
    PupilInfo* pupil = new PupilInfo(pupil_name);
    fire_queen->CheckIn(pupil);
    shell->GetPupils()->Add(pupil);
    if(scenes->First())
      pupil->Add(scenes->First());
  }

  Gled::theOne->SpawnEye(shell, eye_name);
}
