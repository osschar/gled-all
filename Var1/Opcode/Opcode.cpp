//----------------------------------------------------------------------
/*
 *	OPCODE - Optimized Collision Detection
 *	Copyright (C) 2001 Pierre Terdiman
 *	Homepage: http://www.codercorner.com/Opcode.htm
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	Main file for Opcode.dll.
 *	\file		Opcode.cpp
 *	\author		Pierre Terdiman
 *	\date		March, 20, 2001
 */
//----------------------------------------------------------------------

/*
  Finding a good name is difficult!
  Here's the draft for this lib.... Spooky, uh?

  VOID?			Very Optimized Interference Detection
  ZOID?			Zappy's Optimized Interference Detection
  CID?			Custom/Clever Interference Detection
  AID / ACID!		Accurate Interference Detection
  QUID?			Quick Interference Detection
  RIDE?			Realtime Interference DEtection
  WIDE?			Wicked Interference DEtection (....)
  GUID!
  KID !			k-dop interference detection :)
  OPCODE!		OPtimized COllision DEtection
*/

//----------------------------------------------------------------------
// Precompiled Header
#include "Opcode.h"

bool Opcode::InitOpcode()
{
  printf("// Initializing OPCODE\n\n");
  //	LogAPIInfo();
  return true;
}

void ReleasePruningSorters();
bool Opcode::CloseOpcode()
{
  printf("// Closing OPCODE\n\n");

  ReleasePruningSorters();

  return true;
}
