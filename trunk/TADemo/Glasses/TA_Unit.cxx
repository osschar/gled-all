// $Header$
#include "TA_Unit.h"
#include "TA_Unit.c7"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

ClassImp(TA_Unit)

/**************************************************************************/

void TA_Unit::_init()
{
  mFile = "t1hpi/objects3d";
  mTexCont = 0;
  mS = 1; mJ = 0;
}

/**************************************************************************/

TA_Unit::~TA_Unit() {}

void
TA_Unit::Load() {
  if(mFile.Length()==0) return;
  struct stat fs;
  if(stat(mFile, &fs) ) {
    perror("stat failed"); return;
  }
  int size = fs.st_size;
  char* raw_data = new char[size];
  printf("loading %s (%6.2fK)\n", mFile.Data(), ((float)size)/1024);
    
  FILE* fp = fopen(mFile.Data(), "r");
  fread(raw_data, size, 1, fp);
  fclose(fp);
  build(this, (o3dInfo*)raw_data, raw_data);
  { // rename self from base->model name
    char *beg = rindex(mFile.Data(), '/')+1, xx[256];
    int i=0; while(beg[i]!='.') xx[i++] = beg[i]; xx[i] = 0;
    mName = xx;
  }
  Stamp();
  delete [] raw_data;
}
