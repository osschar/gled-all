// $Id$

#include <THtml.h>
#include <TSystem.h>

class ZModDef : public THtml::TModuleDefinition
{
public:
  ZModDef() {}

  virtual bool GetModule(TClass* /*cl*/, THtml::TFileSysEntry* fse,
			 TString& out_modulename) const
  {
    TString modpath(fse->GetParent()->GetParent()->GetName());
    modpath.ReplaceAll("libsets/", "");
    out_modulename = modpath;
    return true;
  }

  ClassDef(ZModDef, 1);
};

class ZHtml : public THtml
{
public:
  ZHtml() : THtml() {}

  void create_list_of_types()
  {
    CreateListOfTypes();
  }

  void create_hierarchy()
  {
    CreateHierarchy();
  }

  void KusKus(const char* className)
  {
    // TClass *classPtr = GetClass(className);
    TClass *classPtr = TClass::GetClass(className);
    TString htmlFile;
    GetHtmlFileName(classPtr, htmlFile);

    // printf("KusKus -- cname=%s, hname=%s\n", className, htmlFile.Data());
    
    if (!htmlFile.IsNull() &&
	(htmlFile.BeginsWith("http://")  ||
	 htmlFile.BeginsWith("https://") ||
	 !gSystem->IsAbsoluteFileName(htmlFile)) )
    {
      MakeClass(className);
      MakeTree(className);
    }
  }

  ClassDef(ZHtml, 1);
}; // endclass ZHtml
