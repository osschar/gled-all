// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GledNS_H
#define Gled_GledNS_H

#include <Gled/GledTypes.h>

class Saturn;
class ZMIR;
class GMutex;

class TMessage;
class TDirectory;
class TBuffer;
class TFile;

namespace GledNS {

  /**************************************************************************/
  // Libset / LibID, ClassID / LibName, ClassName lookup structures
  /**************************************************************************/

  // Demangler/Constructor functions
  typedef Int_t   (*E_DEMANGLE_FOO)    (ZGlass*, TBuffer*);
  typedef ZGlass* (*E_CONSTRUCT_FOO)   (Saturn*, TBuffer*);
  typedef ZGlass* (*DEF_CONSTRUCT_FOO) (CID_t);
  typedef bool 	  (*IS_A_FOO)	       (ZGlass*, CID_t);

  struct LibSetInfo {
    LID_t		fLid;
    string		fName;
    char**		fDeps;
    E_DEMANGLE_FOO	fEDFoo;
    E_CONSTRUCT_FOO	fECFoo;
    DEF_CONSTRUCT_FOO	fDCFoo;
    IS_A_FOO		fISAFoo;
  };

#ifndef __CINT__

  typedef hash_map<LID_t, LibSetInfo*>			hLid2pLSI_t;
  typedef hash_map<LID_t, LibSetInfo*>::iterator	hLid2pLSI_i;

  extern hLid2pLSI_t	Lid2pLSI;

  /**************************************************************************/
  // Hashes of Dependencies and various ID/Name demanglers
  /**************************************************************************/

  typedef hash_map<string, LID_t>		hName2Lid_t;
  typedef hash_map<string, LID_t>::iterator	hName2Lid_i;
  typedef hash_map<string, FID_t>		hName2Fid_t;
  typedef hash_map<string, FID_t>::iterator	hName2Fid_i;

  extern hName2Lid_t	Name2Lid;
  extern hName2Fid_t	Name2Fid;

#endif

  extern TDirectory* GledRoot;
  // current File, Directory
  void InitFD(TFile* file, TDirectory* dir);
  void PushFD();
  void PopFD();

  // Message Types
  enum MessageTypes_e {
    MT_Ray   = 12345,		// Saturn -> Eye
    MT_Beam,			// Directed MIR
    MT_Flare,			// Broadcasted MIR
    MT_God,			// 
    MT_GledProtocol,		// 
    MT_ProtocolMismatch,	// 
    MT_QueryFFID,		// 
    MT_EyeConnect,		// 
    MT_SaturnConnect		// 
  };

  Int_t LoadSoSet(const string& lib_set);
  Int_t InitSoSet(const string& lib_set);
  Int_t LoadSo(const string& full_lib_name);

  //void	BootstrapSoSet(const string& name, LID_t id, E_DEMANGLE_FOO efoo,
  //	       E_CONSTRUCT_FOO ec_foo, DEF_CONSTRUCT_FOO dc_foo,char** deps);
  void	BootstrapSoSet(LibSetInfo* lsi);

  bool	      IsLoaded(const string& lib_set);
  bool	      IsLoaded(LID_t lid);
  LibSetInfo* FindLSI(LID_t lid);

  void  BootstrapClass(const string& name, LID_t l, CID_t c);
  FID_t FindClass(const string& name);

  string FabricateLibName(const string& libset);
  string FabricateInitFoo(const string& libset);
  string FabricateUserInitFoo(const string& libset);

  ZGlass* ConstructGlass(LID_t lid, CID_t cid);
  ZGlass* ConstructGlass(Saturn* s, TBuffer* b);
  bool	  IsA(ZGlass* glass, FID_t fid);

  void    StreamGlass(TBuffer& b, ZGlass* glass);
  ZGlass* StreamGlass(TBuffer& b);
} // namespace GledNS

#endif
