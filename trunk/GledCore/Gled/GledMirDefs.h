// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GledMirDefs_H
#define Gled_GledMirDefs_H

#include <Stones/ZMIR.h>
#include <Ephra/Saturn.h>

#include <memory>

#define SP_MIR(_lens_, _method_, ...) \
  { auto_ptr<ZMIR> mir(_lens_->S_##_method_(__VA_ARGS__)); \
    _lens_->GetSaturn()->PostMIR(*mir); }

#define SP_MIR_BEAM(_caller_, _rec_, _lens_, _method_, ...) \
  { auto_ptr<ZMIR> mir(_lens_->S_##_method_(__VA_ARGS__)); \
    mir->SetCaller(_caller_); mir->SetRecipient(_rec_); \
    _lens_->GetSaturn()->PostMIR(*mir); }

#define SP_MIR_SATURN(_sat_, _lens_, _method_, ...) \
  { auto_ptr<ZMIR> mir(_lens_->S_##_method_(__VA_ARGS__)); \
    _sat_->PostMIR(*mir); }

#endif
