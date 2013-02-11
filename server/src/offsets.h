/*
 *  offsets.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#ifndef OFFSETS_H
#define OFFSETS_H

#include "eiface.h"
#include "server_class.h"

extern IServerGameDLL *serverGameDLL;

//=================================================================================
// Container class for various NetVar offsets
// May expand to hold function offsets when moving to signature scanning (TODO)
// 
// NetVars appended with 'p', functions with 'f' (TODO ^)
// 
// 
// MakePtr, CrawlForPropOffset and WS_FindOffset are based on code by various
// posters from AlliedMods and GameDeception.
// They have been modified to be able to find any property in the hierarchy of
// a class.
//   CTFPlayer example:
//       "m_iHealth" will find CTFPlayer->m_iHealth, 
//    and "m_iClass" will find CTFPlayer->m_PlayerClass->m_iClass
//=================================================================================

class WSOffsets {
public:
	static int pCTFPlayer__m_iClass;
	static int pCWeaponMedigun__m_flChargeLevel;

	static void PrepareOffsets();
	static int FindOffsetOfClassProp(const char *className, const char *propName);

private:
	static bool CrawlForPropOffset(SendTable *sTable, const char *propName, int &offset);
};

#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))

#endif
