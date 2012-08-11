#!/bin/bash

### Make sure we are in the right directory !!!! ###

rm -rf include/
rm -rf lib/*.a
rm -rf lib/*.la

cd root/

rm -rf root/lib/libTMVA.*
rm -rf root/lib/libEve.*
rm -rf root/lib/libRGL.*
rm -rf root/lib/libGeom.*
rm -rf root/lib/libSmatrix.*
rm -rf root/lib/libGeomBuilder.*
rm -rf root/lib/libProof.*
rm -rf root/lib/libProofPlayer.*

rm -rf test/
rm -rf tutorials/

cd cint/cint/lib
find . -name \*.o  | xargs rm
find . -name G__\* | xargs rm
