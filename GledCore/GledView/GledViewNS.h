// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GledViewNS_H
#define Gled_GledViewNS_H

#include <Gled/GledTypes.h>

class ZGlass;
class ZMIR;
class Eye;
class MTW_View;
class MTW_SubView;
class FTW_Ant;
class A_Rnr;
class Fl_Widget;
class TMessage;

namespace GledViewNS {
  /**************************************************************************/
  // Spooky structs & typedefs
  /**************************************************************************/

  // Sub level: Rnr Bits and Ctrl

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
    bool		fRnrLinks;
    UChar_t		fMaxLvl;
    RnrBits		fRnrBits;

    RnrCtrl() : fRnrLinks(false), fMaxLvl(0) {}
    RnrCtrl(bool rl, UChar_t ml, const RnrBits& rb) :
      fRnrLinks(rl), fMaxLvl(ml), fRnrBits(rb) {}
  };

  // Low level: weed info

  typedef Fl_Widget*	(*WeedCreator_foo)    (MTW_SubView*);
  typedef void		(*WeedCallback_foo)   (Fl_Widget*, MTW_SubView*);
  typedef void		(*WeedUpdate_foo)     (Fl_Widget*, MTW_SubView*);
  typedef void		(*CtxCallCreator_foo) (TBuffer*);

  struct MemberInfo {
    string		fName;
    string		fType;
    Int_t		fWidth;
    Int_t		fHeight;
    bool		bLabel;
    bool		bLabelInside;
    bool		bCanResize;
    bool		bJoinNext;
    WeedCreator_foo	fooWCreator;
    WeedCallback_foo	fooWCallback;
    WeedUpdate_foo	fooWUpdate;
  };

  typedef map<string, MemberInfo*>		mName2pMemberInfo_t;
  typedef map<string, MemberInfo*>::iterator	mName2pMemberInfo_i;
  typedef list<MemberInfo*>			lpMemberInfo_t;
  typedef list<MemberInfo*>::iterator		lpMemberInfo_i;

  struct ContextMethodInfo {
    string		fName;
    lStr_t		fContextArgs;
    lStr_t		fFreeArgs;
    string		fFreeTemplate;
    CtxCallCreator_foo	fooCCCreator;

    bool operator==(const string& s) { return (fName == s); }
  };

  typedef list<ContextMethodInfo*>		lpContextMethodInfo_t;
  typedef list<ContextMethodInfo*>::iterator	lpContextMethodInfo_i;

  struct LinkMemberInfo {
    string		fName;
    string		fType;
    bool		bIsLinkToList;
    RnrBits		fDefRnrBits;
    CtxCallCreator_foo	fooCCCreator;
  };

  typedef list<LinkMemberInfo*>			lpLinkMemberInfo_t;
  typedef list<LinkMemberInfo*>::iterator	lpLinkMemberInfo_i;

  //typedef hash_map<CID_t, mName2pMemberInfo_t*>		hCid2pMImap_t;
  //typedef hash_map<CID_t, mName2pMemberInfo_t*>::iterator	hCid2pMImap_i;
  //typedef hash_map<CID_t, lpMemberInfo_t*>			hCid2pMIlist_t;
  //typedef hash_map<CID_t, lpMemberInfo_t*>::iterator		hCid2pMIlist_i;

  // Intermediate level: ClassInfo, RnrCreator and SubView info

  struct ClassInfo;
  typedef MTW_SubView*	(*SubViewCreator_foo)(ClassInfo* ci, MTW_View* v, ZGlass* g);

  struct ClassInfo {
    FID_t			fFid;
    string			fClassName;
    SubViewCreator_foo		fooSVCreator;
    mName2pMemberInfo_t		fMImap;
    lpMemberInfo_t		fMIlist;
    lpContextMethodInfo_t	fCMIlist;
    lpLinkMemberInfo_t		fLMIlist;
    string			fParentName;
    string			fRendererGlass;
    RnrCtrl			fDefRnrCtrl;
    ClassInfo*			fRendererCI;
    ClassInfo*			fParentCI;

    ClassInfo(FID_t f) : fFid(f) {} // all defed by p7

    lpMemberInfo_t&     GetMemberInfoList()     { return fMIlist; }
    lpMemberInfo_t*	ProduceFullMemberInfoList();
    lpLinkMemberInfo_t& GetLinkMemberInfoList() { return fLMIlist; }
    lpLinkMemberInfo_t*	ProduceFullLinkMemberInfoList();
    MemberInfo*		FindMemberInfo(const string& mmb);
    ContextMethodInfo*	FindContextMethodInfo(const string& func_name);

    ClassInfo*		GetParentCI();
    ClassInfo*		GetRendererCI();
    A_Rnr*		SpawnRnr(const string& rnr, ZGlass* g);
  };

  typedef A_Rnr*	(A_Rnr_Creator_foo)	(ZGlass*, CID_t);

  typedef hash_map<string, A_Rnr_Creator_foo*>		 hRnr2RCFoo_t;
  typedef hash_map<string, A_Rnr_Creator_foo*>::iterator hRnr2RCFoo_i;
  typedef hash_map<CID_t, ClassInfo*>			 hCid2pCI_t;
  typedef hash_map<CID_t, ClassInfo*>::iterator		 hCid2pCI_i;

  // High level: per LibSet information

  struct LibSetInfo {
    LID_t		fLid;
    string		fName;
    hCid2pCI_t		Cid2pCI;
    hRnr2RCFoo_t	Rnr2RCFoo;
  
    LibSetInfo(LID_t lid, const string& s) : fLid(lid), fName(s) {}
    ClassInfo* FindClassInfo(CID_t cid);
    ClassInfo* FirstClassInfo();
  };

  typedef hash_map<LID_t, LibSetInfo>		hLid2LSInfo_t;
  typedef hash_map<LID_t, LibSetInfo>::iterator	hLid2LSInfo_i;

  /**************************************************************************/
  // Variables
  /**************************************************************************/

  extern hLid2LSInfo_t	Lid2LSInfo;
  extern set<string>	RnrNames;

  /**************************************************************************/
  // Functions
  /**************************************************************************/

  Int_t LoadSoSet(const string& lib_set);
  Int_t InitSoSet(const string& lib_set);
  void	BootstrapViewSet(LID_t lid, const string& libset);
  void  BootstrapClassInfo(ClassInfo* c_info);
  void  BootstrapRnrSet(const string& libset, LID_t id,
			const string& rnr,  A_Rnr_Creator_foo rfoo);

  string FabricateViewLibName(const string& libset);
  string FabricateViewInitFoo(const string& libset);
  string FabricateViewUserInitFoo(const string& libset);
  string FabricateRnrLibName(const string& libset, const string& rnr);
  string FabricateRnrInitFoo(const string& libset, const string& rnr);

  void AssertRenderers();
  void AddRenderer(const string& rnr);
  
  // Inquiries

  LibSetInfo* FindLibSetInfo(LID_t lid);
  ClassInfo*  FindClassInfo(FID_t fid);

  // Services

  A_Rnr* SpawnRnr(const string& rnr, ZGlass* d, LID_t lid, CID_t cid);
} // GledViewNS

#endif
