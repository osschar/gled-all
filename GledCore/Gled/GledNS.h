// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GledNS_H
#define Gled_GledNS_H

#include <Gled/GledTypes.h>

class Saturn;
class SaturnInfo;
class ZMIR;
class GMutex;

class TMessage;
class TDirectory;
class TBuffer;
class TFile;

namespace GledViewNS {
  class MethodInfo;
  class DataMemberInfo;
  class LinkMemberInfo;
  class ClassInfo;
  class LibSetInfo;
}

namespace GledNS {

  /**************************************************************************/
  // Elements of ClassInfo: Method, DataMember & LinkMember Infos
  /**************************************************************************/

  class ClassInfo;

  struct InfoBase {
    string		fName;

    InfoBase(const string& s) : fName(s) {}
    bool operator==(const string& s) const { return (fName == s); }
  };

  struct MethodInfo : public InfoBase {
    MID_t		fMid;
    lStr_t		fContextArgs; //
    lStr_t		fArgs;
    lStr_t		fTags;
    bool		bLocal;
    bool		bDetachedExe;
    bool		bMultixDetachedExe;
    ClassInfo*		fClassInfo;

    GledViewNS::MethodInfo* fViewPart;

    MethodInfo(const string& s, MID_t m) :
      InfoBase(s), fMid(m),
      bDetachedExe(false), bMultixDetachedExe(false),
      fViewPart(0) {}
    void ImprintMir(ZMIR& mir) const;
    void FixMirBits(ZMIR& mir, SaturnInfo* sat) const;
    void StreamIds(TBuffer& b) const;
  };


  struct DataMemberInfo : public InfoBase {
    string		fType;
    MethodInfo*		fSetMethod;

    GledViewNS::DataMemberInfo* fViewPart;

    DataMemberInfo(const string& s) : InfoBase(s), fViewPart(0) {}
  };

  struct LinkMemberInfo : public InfoBase {
    string		fType;
    MethodInfo*		fSetMethod;

    GledViewNS::LinkMemberInfo* fViewPart;

    LinkMemberInfo(const string& s) : InfoBase(s), fViewPart(0) {}
  };

  // List typedefs

  typedef list<MethodInfo*>			lpMethodInfo_t;
  typedef list<MethodInfo*>::iterator		lpMethodInfo_i;

  typedef list<DataMemberInfo*>			lpDataMemberInfo_t;
  typedef list<DataMemberInfo*>::iterator	lpDataMemberInfo_i;

  typedef list<LinkMemberInfo*>			lpLinkMemberInfo_t;
  typedef list<LinkMemberInfo*>::iterator	lpLinkMemberInfo_i;

#ifndef __CINT__
  typedef hash_map<MID_t, MethodInfo*>           hMid2pMethodInfo_t;
  typedef hash_map<MID_t, MethodInfo*>::iterator hMid2pMethodInfo_i;
#endif

  /**************************************************************************/
  /**************************************************************************/

  struct ClassInfo : public InfoBase {
    FID_t			fFid;
    string			fParentName;
    ClassInfo*			fParentCI;

    lpMethodInfo_t		fMethodList;
    lpDataMemberInfo_t		fDataMemberList;
    lpLinkMemberInfo_t		fLinkMemberList;
#ifndef __CINT__
    hMid2pMethodInfo_t		fMethodHash;
#endif

    GledViewNS::ClassInfo*	fViewPart;

    //----------------------------------------------------------------

    ClassInfo(const string& s, FID_t f) : InfoBase(s), fFid(f), fViewPart(0) {}

    lpDataMemberInfo_t*	ProduceFullDataMemberInfoList();
    lpLinkMemberInfo_t*	ProduceFullLinkMemberInfoList();

    MethodInfo*		FindMethodInfo(MID_t mid);
    MethodInfo*		FindMethodInfo(const string& func_name, bool recurse);
    DataMemberInfo*	FindDataMemberInfo(const string& mmb, bool recurse);
    LinkMemberInfo*	FindLinkMemberInfo(const string& mmb, bool recurse);

    ClassInfo*		GetParentCI();
  };

  /**************************************************************************/
  /**************************************************************************/

#ifndef __CINT__
  typedef hash_map<CID_t, ClassInfo*>			 hCid2pCI_t;
  typedef hash_map<CID_t, ClassInfo*>::iterator		 hCid2pCI_i;
#endif

  /**************************************************************************/
  // Libset / LibID, ClassID / LibName, ClassName lookup structures
  /**************************************************************************/

  // Demangler/Constructor functions
  typedef void    (*Libset_Mir_Exec_foo)  (ZGlass*, ZMIR&);
  typedef ZGlass* (*Lens_Constructor_foo) (CID_t);
  typedef bool 	  (*Is_A_Glass_foo)	  (ZGlass*, CID_t);

  struct LibSetInfo : public InfoBase {
    LID_t		fLid;
    char**		fDeps;

#ifndef __CINT__
    hCid2pCI_t		Cid2CInfo;
#endif

    Libset_Mir_Exec_foo		fLME_Foo;
    Lens_Constructor_foo	fLC_Foo;
    Is_A_Glass_foo		fISA_Foo;

    GledViewNS::LibSetInfo*	fViewPart;

    //--------------------------------------------------------------

    LibSetInfo(const string& s, LID_t lid) : InfoBase(s), fLid(lid), fViewPart(0) {}

    ClassInfo* FindClassInfo(CID_t cid);
    ClassInfo* FirstClassInfo();
  };

  typedef list<LibSetInfo*>				lpLSI_t;
  typedef list<LibSetInfo*>::iterator			lpLSI_i;

#ifndef __CINT__

  typedef hash_map<LID_t, LibSetInfo*>			hLid2pLSI_t;
  typedef hash_map<LID_t, LibSetInfo*>::iterator	hLid2pLSI_i;

  /**************************************************************************/
  // Hashes of Dependencies and various ID/Name demanglers
  /**************************************************************************/

  typedef hash_map<string, LID_t>		hName2Lid_t;
  typedef hash_map<string, LID_t>::iterator	hName2Lid_i;
  typedef hash_map<string, FID_t>		hName2Fid_t;
  typedef hash_map<string, FID_t>::iterator	hName2Fid_i;

#endif

  // Inquiries

  LibSetInfo* FindLibSetInfo(LID_t lid);
  LibSetInfo* FindLibSetInfo(const string& lib_set);
  void	      ProduceLibSetInfoList(lpLSI_t& li_list);
  ClassInfo*  FindClassInfo(FID_t fid);
  ClassInfo*  FindClassInfo(const string& name);
  FID_t       FindClassID(const string& name);

  /**************************************************************************/
  /**************************************************************************/

  extern TDirectory* GledRoot;
  // current File, Directory
  void InitFD(TFile* file, TDirectory* dir);
  void PushFD();
  void PopFD();

  // Message Types
  enum MessageTypes_e {
    // MIR types
    MT_Beam            = 12000, // Directed MIR
    MT_Flare,			// Broadcasted MIR

    // Saturn, Eye -> Saturn communication
    MT_GledProtocol    = 12010, //
    MT_ProtocolMismatch,	//
    MT_QueryFFID,		//
    MT_MEE_Connect,		//
    MT_MEE_Authenticate,	//

    MT_MEE_ConnectionGranted,	// Followed by MEE specific data
    MT_MEE_AuthRequested,	// Followed by hostname, port, conn_id
    MT_MEE_ConnectionDenied,	// Followed by string

    MT_Auth_Challenge,		// challenge + pub key of SunAbsolute
    MT_Auth_ChallengeResponse,	// challenge response

    // Saturn -> Eye communication
    MT_Ray             = 12050,	// Lens changes
    MT_TextMessage,             // Messages, Errors, Exceptions
    MT_EyeCommand               // Commands for the Eye
  };

  Int_t LoadSoSet(const string& lib_set);
  Int_t InitSoSet(const string& lib_set);
  Int_t LoadSo(const string& full_lib_name);

  //void	BootstrapSoSet(const string& name, LID_t id, E_DEMANGLE_FOO efoo,
  //	       E_CONSTRUCT_FOO ec_foo, DEF_CONSTRUCT_FOO dc_foo,char** deps);
  void	BootstrapSoSet(LibSetInfo* lsi);

  bool	IsLoaded(const string& lib_set);
  bool	IsLoaded(LID_t lid);

  void  BootstrapClass(ClassInfo* ci);

  string FabricateLibName(const string& libset);
  string FabricateInitFoo(const string& libset);
  string FabricateUserInitFoo(const string& libset);

  ZGlass* ConstructLens(LID_t lid, CID_t cid);
  bool	  IsA(ZGlass* glass, FID_t fid);

  void    LockCINT();
  void    UnlockCINT();

  void    StreamLens(TBuffer& b, ZGlass* lens);
  ZGlass* StreamLens(TBuffer& b);

  template <class GLASS>
  GLASS StreamLensByGlass(TBuffer& b) {
    ZGlass* lens = StreamLens(b);
    if(lens) { 
      GLASS ret = dynamic_cast<GLASS>(lens);
      if(ret == 0) delete lens;
      return ret;
    } else {
      return 0;
    }
  }


  /**************************************************************************/
  // Simple string parser
  /**************************************************************************/

#ifndef __CINT__

  int  split_string(Str_ci start, Str_ci end, lStr_t& l, char c=0);
  int  split_string(const string& s, lStr_t& l, char c=0);
  int  split_string(const string& s, lStr_t& l, string ptr);
  void deparen_string(const string& in, string& n, string& a,
		      const string& ops="([{", bool no_parens_ok=false)
    throw(string);

  // url_tokenizator

  class url_token : public string {
  public:
    enum type_e { null, link_sel, list_sel };
  protected:
    type_e m_type;
  public:
    url_token(type_e t=null) : string(), m_type(t) {}
    url_token(const string& s, type_e t=null) : string(s), m_type(t) {}

    type_e type() const { return m_type; }
    void type(type_e t) { m_type = t; }
  };

  int tokenize_url(const string& url, list<url_token>& l);

#endif

} // namespace GledNS

#endif
