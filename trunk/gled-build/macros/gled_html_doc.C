// $Header$

#include <THtml.h>

class ZHtml : public THtml {

public:
  ZHtml() : THtml() {}

  void gen_list_of_types() { CreateListOfTypes(); }

  ClassDef(ZHtml, 1)
}; // endclass ZHtml

ClassImp(ZHtml)
