// $Id$

#ifndef RT_FUNCTIONS_H
#define RT_FUNCTIONS_H

#include <Rtypes.h>

class ZNode;
class Eventor;
class RndSMorphCreator;

extern ZNode**           subnodes;
extern Eventor*          eventor;
extern RndSMorphCreator* smorph_creator;
extern ZNode*            top_node;

extern void create_smorphs(Int_t NSN = 32, Int_t NNN = 1024, Int_t SMS = 0);

#endif
