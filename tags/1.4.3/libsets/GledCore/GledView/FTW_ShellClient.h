// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_ShellClient_H
#define GledCore_FTW_ShellClient_H

class FTW_Shell;

class FTW_ShellClient
{
protected:
  FTW_Shell*    mShell;

public:
  FTW_ShellClient(FTW_Shell* s);

  FTW_Shell* GetShell() { return mShell; }
};

#endif
