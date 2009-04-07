// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GledNS_H
#define GledCore_GledNS_H

#include <Gled/GledTypes.h>

class Saturn;
class SaturnInfo;
class ZMIR;
class A_Rnr;
class GMutex;

class TMessage;
class TDirectory;
class TBuffer;
class TFile;
class TVirtualMutex;

class TClass;
class TRealData;
class TDataMember;

namespace GledViewNS {
  class MethodInfo;
  class DataMemberInfo;
  class LinkMemberInfo;
  class ClassInfo;
  class LibSetInfo;
}

namespace GledNS {

  /**************************************************************************/
  // Rnr Bits and Ctrl
  /**************************************************************************/
  // Or move them to RnrBase something ...

  struct RnrBits {
    // At which RenderLevel to call a specific rendering
    // Values of fX are RenderLevels at which to invoke the method
    // 0 ~ PreDraw, 1 ~ Draw, 2 ~ PostDraw; 3 ~ FullDescent
    UChar_t	fSelf[4];	// Self rendering
    UChar_t	fList[4];	// List member rendering

    RnrBits(UChar_t a=0, UChar_t b=0, UChar_t c=0, UChar_t d=0,
	    UChar_t x=0, UChar_t y=0, UChar_t w=0, UChar_t z=0) {
      fSelf[0] = a; fSelf[1] = b; fSelf[2] = c; fSelf[3] = d;
      fList[0] = x; fList[1] = y; fList[2] = w; fList[3] = z;
    }
    void SetSelf(UChar_t a=0, UChar_t b=0, UChar_t c=0, UChar_t d=0) {
      fSelf[0] = a; fSelf[1] = b; fSelf[2] = c; fSelf[3] = d;
    }
    void SetList(UChar_t x=0, UChar_t y=0, UChar_t w=0, UChar_t z=0) {
      fList[0] = x; fList[1] = y; fList[2] = w; fList[3] = z;
    }

    bool SelfOn() const { return fSelf[0]!=0 || fSelf[1]!=0 || fSelf[2]!=0 || fSelf[3]!=0; }
    bool SelfOnDirect() const { return fSelf[0]!=0 || fSelf[1]!=0 || fSelf[2]!=0; }
    bool ListOn() const { return fList[0]!=0 || fList[1]!=0 || fList[2]!=0 || fList[3]!=0; }
    bool ListOnDirect() const { return fList[0]!=0 || fList[1]!=0 || fList[2]!=0; }
  };

  struct RnrCtrl {
    RnrBits		fRnrBits;

    RnrCtrl() : fRnrBits(2,4,6,0, 0,0,0,5) {}
    RnrCtrl(const RnrBits& rb) : fRnrBits(rb) {}
  };


  /**************************************************************************/
  // Elements of ClassInfo: Method, DataMember & LinkMember Infos
  /**************************************************************************/

  class ClassInfo;
  class LibSetInfo;

  struct InfoBase {
    TString		fName;
    ClassInfo*		fClassInfo;

    InfoBase(const TString& s) : fName(s), fClassInfo(0) {}
    bool operator==(const TString& s) const { return (fName == s); }
  };

  struct MethodInfo : public InfoBase {
    MID_t		fMid;
    lStr_t		fContextArgs;
    lStr_t		fArgs;
    lStr_t		fTags;
    bool		bLocal;
    bool		bDetachedExe;
    bool		bMultixDetachedExe;

    MethodInfo(const TString& s, MID_t m) :
      InfoBase(s), fMid(m),
      bDetachedExe(false), bMultixDetachedExe(false)
    {}
    ZMIR* MakeMir(ZGlass* a, ZGlass* b=0, ZGlass* g=0);
    void  ImprintMir(ZMIR& mir) const;
    void  StreamIds(TBuffer& b) const;
  };

  struct DataMemberInfo : public InfoBase {
    TString             fPrefix;
    TString		fType;
    MethodInfo*		fSetMethod;
    TRealData*          fTRealData;

    DataMemberInfo(const TString& s) :
      InfoBase(s), fSetMethod(0), fTRealData(0) {}

    TString CName();
    TString FullName();
    TString FullCName();

    TRealData*   GetTRealData();
    TDataMember* GetTDataMember();
  };

  struct LinkMemberInfo : public DataMemberInfo {
    RnrBits		fDefRnrBits;

    LinkMemberInfo(const TString& s) : DataMemberInfo(s) {}
  };

  struct EnumEntry
  {
    TString   fName;
    TString   fLabel;
    Int_t     fValue;

    EnumEntry(const TString& n, const TString& l, Int_t v) :
      fName(n), fLabel(l), fValue(v) {}
  };

  typedef vector<EnumEntry>           vEnumEntry_t;
  typedef vector<EnumEntry>::iterator vEnumEntry_i;

  struct EnumInfo : public InfoBase
  {
    vEnumEntry_t  fEntries;
    Int_t         fMaxLabelWidth;

    EnumInfo(const TString& s, Int_t size);

    void AddEntry(const TString& n, const TString& l, Int_t v);

    EnumEntry* FindEntry(const TString& name);
  };


  // List typedefs

  typedef list<MethodInfo*>			lpMethodInfo_t;
  typedef list<MethodInfo*>::iterator		lpMethodInfo_i;

  typedef list<DataMemberInfo*>			lpDataMemberInfo_t;
  typedef list<DataMemberInfo*>::iterator	lpDataMemberInfo_i;

  typedef list<LinkMemberInfo*>			lpLinkMemberInfo_t;
  typedef list<LinkMemberInfo*>::iterator	lpLinkMemberInfo_i;

  typedef list<EnumInfo*>			lpEnumInfo_t;
  typedef list<EnumInfo*>::iterator		lpEnumInfo_i;

#ifndef __CINT__
  typedef hash_map<MID_t, MethodInfo*>           hMid2pMethodInfo_t;
  typedef hash_map<MID_t, MethodInfo*>::iterator hMid2pMethodInfo_i;
#endif


  /**************************************************************************/
  /**************************************************************************/

  struct ClassInfo : public InfoBase {
    FID_t			fFid;
    LibSetInfo*			fLibSetInfo;

    TString			fParentName;
    ClassInfo*			fParentCI;

    lpMethodInfo_t		fMethodList;
    lpDataMemberInfo_t		fDataMemberList;
    lpLinkMemberInfo_t		fLinkMemberList;
    lpEnumInfo_t		fEnumList;
#ifndef __CINT__
    hMid2pMethodInfo_t		fMethodHash;
#endif

    TString			fRendererGlass;
    RnrCtrl			fDefRnrCtrl;
    ClassInfo*	                fRendererCI;

    GledViewNS::ClassInfo*	fViewPart;

    //----------------------------------------------------------------

    ClassInfo(const TString& s, FID_t f) :
      InfoBase(s),
      fFid(f), fLibSetInfo(0), fParentCI(0),
      fRendererCI(0),
      fViewPart(0),
      fTClass(0)
    {}

    lpDataMemberInfo_t*	ProduceFullDataMemberInfoList();
    lpLinkMemberInfo_t*	ProduceFullLinkMemberInfoList();

    MethodInfo*		FindMethodInfo    (MID_t mid);
    MethodInfo*		FindMethodInfo    (const TString& name, bool recurse, bool throwp=true);
    DataMemberInfo*	FindDataMemberInfo(const TString& name, bool recurse, bool throwp=true);
    LinkMemberInfo*	FindLinkMemberInfo(const TString& name, bool recurse, bool throwp=true);
    EnumInfo*           FindEnumInfo      (const TString& name, bool recurse, bool throwp=true);

    LibSetInfo*		GetLibSetInfo();
    ClassInfo*		GetParentCI();

    ClassInfo*          GetRendererCI();
    A_Rnr*              SpawnRnr(const TString& rnr, ZGlass* g);

    TClass* fTClass;
    TClass* GetTClass();
  };

  /**************************************************************************/
  // LibSet
  /**************************************************************************/

  // Demangler/Constructor functions
  typedef void    (*Libset_Mir_Exec_foo)  (ZGlass*, ZMIR&);
  typedef ZGlass* (*Lens_Constructor_foo) (CID_t);
  typedef bool 	  (*Is_A_Glass_foo)	  (ZGlass*, CID_t);
  typedef A_Rnr*  (*A_Rnr_Creator_foo)    (ZGlass*, CID_t);

#ifndef __CINT__
  typedef hash_map<CID_t, ClassInfo*>			 hCid2pCI_t;
  typedef hash_map<CID_t, ClassInfo*>::iterator		 hCid2pCI_i;

  typedef hash_map<TString, A_Rnr_Creator_foo>		 hRnr2RCFoo_t;
  typedef hash_map<TString, A_Rnr_Creator_foo>::iterator  hRnr2RCFoo_i;
#endif

  struct LibSetInfo : public InfoBase {
    LID_t		fLid;
    const char**	fDeps;

#ifndef __CINT__
    hCid2pCI_t		Cid2CInfo;
    hRnr2RCFoo_t	Rnr2RCFoo;
#endif

    Libset_Mir_Exec_foo		fLME_Foo;
    Lens_Constructor_foo	fLC_Foo;
    Is_A_Glass_foo		fISA_Foo;

    GledViewNS::LibSetInfo*	fViewPart;

    //--------------------------------------------------------------

    LibSetInfo(const TString& s, LID_t lid) :
      InfoBase(s), fLid(lid) , fViewPart(0)
    {}

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

  typedef hash_map<TString, LID_t>		hName2Lid_t;
  typedef hash_map<TString, LID_t>::iterator	hName2Lid_i;
  typedef hash_map<TString, FID_t>		hName2Fid_t;
  typedef hash_map<TString, FID_t>::iterator	hName2Fid_i;

#endif

  // Inquiries

  LibSetInfo* FindLibSetInfo(LID_t lid);
  LibSetInfo* FindLibSetInfo(const TString& lib_set);
  void	      ProduceLibSetInfoList(lpLSI_t& li_list);
  ClassInfo*  FindClassInfo(FID_t fid);
  ClassInfo*  FindClassInfo(const TString& name);
  FID_t       FindClassID(const TString& name);

  MethodInfo*     DeduceMethodInfo(ZGlass* alpha, const TString& name);
  DataMemberInfo* DeduceDataMemberInfo(ZGlass* alpha, const TString& name);

  // Renderers

  extern set<TString>	RnrNames;

  void BootstrapRnrSet(const TString& libset, LID_t id,
		       const TString& rnr,  A_Rnr_Creator_foo rfoo);

  TString FabricateRnrLibName(const TString& libset, const TString& rnr);
  TString FabricateRnrInitFoo(const TString& libset, const TString& rnr);

  void AssertRenderers();
  void AddRenderer(const TString& rnr);

  A_Rnr* SpawnRnr(const TString& rnr, ZGlass* d, FID_t fid);

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
    MT_MEE_ConnectionDenied,	// Followed by TString

    MT_Auth_Challenge,		// challenge + pub key of SunAbsolute
    MT_Auth_ChallengeResponse,	// challenge response

    // Saturn -> Eye communication
    MT_Ray             = 12050,	// Lens changes
    MT_TextMessage,             // Messages, Errors, Exceptions
    MT_EyeCommand               // Commands for the Eye
  };

  Int_t LoadSoSet(const TString& lib_set);
  Int_t InitSoSet(const TString& lib_set);
  Int_t LoadSo(const TString& full_lib_name);
  void* FindSymbol(const TString& sym);

  void	BootstrapSoSet(LibSetInfo* lsi);

  bool	IsLoaded(const TString& lib_set);
  bool	IsLoaded(LID_t lid);

  void  BootstrapClass(ClassInfo* ci);

  TString FabricateLibName(const TString& libset);
  TString FabricateInitFoo(const TString& libset);
  TString FabricateUserInitFoo(const TString& libset);

  ZGlass* ConstructLens(FID_t fid);
  bool	  IsA(ZGlass* glass, FID_t fid);

  TVirtualMutex* GetCINTMutex();
  void    LockCINT();
  void    UnlockCINT();

  ZGlass* CastID2Lens(ID_t id);
  ID_t    CastLens2ID(ZGlass* lens);
  void*   CastID2VoidPtr(ID_t id);
  ID_t    CastVoidPtr2ID(void* ptr);

  void    StreamLens(TBuffer& b, ZGlass* lens);
  ZGlass* StreamLens(TBuffer& b);
  void    WriteLensID(TBuffer& b, ZGlass* lens);
  ID_t    ReadLensID(TBuffer& b);
  ZGlass* ReadLensIDAsPtr(TBuffer& b);

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
  // Value-type peek and MIR-poke
  /**************************************************************************/

  Double_t peek_value(void* addr, Int_t type);
  void     stream_value(TBuffer& b, Int_t type, Double_t value);

  /**************************************************************************/
  // Simple TString parser
  /**************************************************************************/

#ifndef __CINT__

  int  split_string(const TString& s, Ssiz_t start, Ssiz_t end,
		    lStr_t& l, char c=0);
  int  split_string(const TString& s, lStr_t& l, char c=0);
  int  split_string(const TString& s, lStr_t& l, const TString& ptr);
  void deparen_string(const TString& in, TString& n, TString& a,
		      const TString& ops="([{", bool no_parens_ok=false)
    throw(Exc_t);

  TString join_strings(const TString& sep, lStr_t& list);

  void   remove_whitespace(TString& s);
  Ssiz_t find_first_of(const TString& s, const char* accept, Ssiz_t i=0);

  // Function argument parsing, type converters

  void split_argument(const TString& arg,
		      TString& type, TString& name, TString& def);
  void unrefptrconst_type(TString& type);

  // url_tokenizator

  class url_token : public TString {
  public:
    enum type_e { null, link_sel, list_sel };
  protected:
    type_e m_type;
  public:
    url_token(type_e t=null) : TString(), m_type(t) {}
    url_token(const TString& s, type_e t=null) : TString(s), m_type(t) {}

    type_e type() const { return m_type; }
    void type(type_e t) { m_type = t; }
  };

  int tokenize_url(const TString& url, list<url_token>& l);

#endif


/**************************************************************************/
// Inlines
/**************************************************************************/

inline ZGlass* CastID2Lens(ID_t id)
{
  char* ptr = 0; ptr += id;
  return (ZGlass*)ptr;
}

inline ID_t CastLens2ID(ZGlass* lens)
{
  return (ID_t) ((char*)lens - (char*)0);
}

inline void* CastID2VoidPtr(ID_t id)
{
  char* ptr = 0; ptr += id;
  return (void*)ptr;
}

inline ID_t CastVoidPtr2ID(void* ptr)
{
  return (ID_t) ((char*)ptr - (char*)0);
}

} // namespace GledNS

#endif
