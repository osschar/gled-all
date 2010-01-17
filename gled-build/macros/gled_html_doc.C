// $Id$

#include <THtml.h>
#include <TSystem.h>

class ZModDef : public THtml::TModuleDefinition
{
public:
  TString fModule;

  ZModDef() {}

  virtual bool GetModule(TClass* cl, THtml::TFileSysEntry* fse,
			 TString& out_modulename) const
  {
    out_modulename = fModule;
    return true;
  }

  ClassDef(ZModDef, 1);
};

class ZHtml : public THtml
{
public:
  ZHtml() : THtml() {}

  void gen_list_of_types()
  { CreateListOfTypes(); }

  void create_hierarchy(const char** classNames, Int_t numberOfClasses)
  {
    // This does not exist any more. Axel?
    // CreateHierarchy(classNames, numberOfClasses);
    CreateHierarchy();
  }

  void create_index(const char** classNames, Int_t numberOfClasses)
  {
    // CreateIndex(classNames, numberOfClasses);
    MakeIndex();
  }

  void KusKus(const char* className)
  {
    // TClass *classPtr = GetClass(className);
    TClass *classPtr = TClass::GetClass(className);
    TString htmlFile;
    GetHtmlFileName(classPtr, htmlFile);

    printf("KusKus -- cname=%s, hname=%s\n", className, htmlFile.Data());
    
    if (!htmlFile.IsNull() &&
	(htmlFile.BeginsWith("http://")  ||
	 htmlFile.BeginsWith("https://") ||
	 !gSystem->IsAbsoluteFileName(htmlFile)) )
    {
      // Class2Html(classPtr);
      MakeClass(className);
      MakeTree(className);
    }
  }

  ClassDef(ZHtml, 1);
}; // endclass ZHtml
