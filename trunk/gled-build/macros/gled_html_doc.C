// $Header$

#include <THtml.h>
#include <TSystem.h>

#include <string.h>

class ZHtml : public THtml {

public:
  ZHtml() : THtml() {}

  void gen_list_of_types()
  { CreateListOfTypes(); }

  void create_hierarchy(const char** classNames, Int_t numberOfClasses)
  { CreateHierarchy(classNames, numberOfClasses); }

  void create_index(const char** classNames, Int_t numberOfClasses)
  { CreateIndex(classNames, numberOfClasses); }

  void KusKus(const char* className);

  ClassDef(ZHtml, 1)
}; // endclass ZHtml

ClassImp(ZHtml)

void ZHtml::KusKus(const char* className)
{
   TClass *classPtr = GetClass(className);
   char *htmlFile = GetHtmlFileName(classPtr);
   if (htmlFile && (strncmp(htmlFile, "http://", 7)  ||
		    strncmp(htmlFile, "https://", 8) ||
		    !gSystem->IsAbsoluteFileName(htmlFile)) )
     {
       Class2Html(classPtr);
       MakeTree(className);
     }
   delete[]htmlFile;
}
