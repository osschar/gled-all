// $Id$
// #define macros for construction of object graphs from INTERPRETED
// cint scripts

#ifndef GLASS_DEFINES_H
#define GLASS_DEFINES_H

#define CREATE_GLASS(_var_, _glass_, _queen_, _name_, _title_) \
  _glass_* _var_ = new _glass_(_name_, _title_); \
  _queen_->CheckIn(_var_)

#define ASSIGN_GLASS(_var_, _glass_, _queen_, _name_, _title_) \
  _var_ = new _glass_(_name_, _title_); \
  _queen_->CheckIn(_var_)

/**************************************************************************/

#define CREATE_ADD_GLASS(_var_, _glass_, _list_, _name_, _title_) \
  _glass_* _var_ = new _glass_(_name_, _title_); \
  _list_->GetQueen()->CheckIn(_var_); _list_->Add(_var_)

#define CREATE_ATT_GLASS(_var_, _glass_, _tgt_, _att_, _name_, _title_) \
  _glass_* _var_ = new _glass_(_name_, _title_); \
  _tgt_->GetQueen()->CheckIn(_var_); _tgt_->_att_(_var_)

/**************************************************************************/

#define ASSIGN_ADD_GLASS(_var_, _glass_, _list_, _name_, _title_) \
  _var_ = new _glass_(_name_, _title_); \
  _list_->GetQueen()->CheckIn(_var_); _list_->Add(_var_)

#define ASSIGN_ATT_GLASS(_var_, _glass_, _tgt_, _att_, _name_, _title_) \
  _var_ = new _glass_(_name_, _title_); \
  _tgt_->GetQueen()->CheckIn(_var_); _tgt_->_att_(_var_)

/**************************************************************************/

#define PATH_FIND_GLASS(_var_, _glass_, _lens_, _path_) \
  _glass_* _var_ = _lens_->FindLensByPath(_path_); \
  if(_var_ && GledNS::IsA(_var_, _glass_::FID())==false) _var_ = 0

#endif
