// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_GledOperatorDefs_H
#define GledCore_GledOperatorDefs_H

#include <Gled/GledMirDefs.h>

#define OP_EXE_OR_SP_MIR(_lens_, _method_, ...) { \
    if(op_arg->fMultix) { _lens_->_method_(__VA_ARGS__); } \
    else                { SP_MIR(_lens_, _method_, __VA_ARGS__) }}

#define OP_EXE_OR_SP_MIR_SATURN(_sat_, _lens_, _method_, ...) { \
    if(op_arg->fMultix) { _lens_->_method_(__VA_ARGS__); } \
    else                { SP_MIR_SATURN(_sat_, _lens_, _method_, __VA_ARGS__) }}

#endif
