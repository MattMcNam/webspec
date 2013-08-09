/*
 *  vfuncs.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#ifndef VFUNCS_H
#define VFUNCS_H

#include <stdint.h>

class WSEmptyClass {};

class CBaseEntity;
class CBaseCombatWeapon;
class CBaseCombatCharacter;

//=================================================================================
// Virtual function indexes
// 
// If anything is likely to break, it's in here, but unfortunately it seems to be
// the only way to do certain things.
// 
// Indexes are obtained via an IDA script provided by AlliedModders. Looking to
// switch to signature scanning however.
//  Current CBaseEntity indexes: https://dl.dropbox.com/u/6635591/links/vfunc-indexes-CBaseEntity.txt
//  Current CBaseCombatCharacter indexes: https://dl.dropbox.com/u/6635591/links/vfunc-indexes-CBaseCombatCharacter.txt
//  -Linux indexes, Windows is _usually_ 1 less
//=================================================================================

#if defined(_LINUX) || defined(_OSX)
#define VFUNCINDEX_CBaseCombatCharacter_Weapon_GetSlot 268
#define VFUNCINDEX_CBaseEntity_MyCombatCharacterPointer 72
#else
#define VFUNCINDEX_CBaseCombatCharacter_Weapon_GetSlot 267
#define VFUNCINDEX_CBaseEntity_MyCombatCharacterPointer 71
#endif

extern CBaseCombatWeapon *CBaseCombatCharacter_Weapon_GetSlot(CBaseCombatCharacter *pThisPtr, int slot);

extern CBaseCombatCharacter *CBaseEntity_MyCombatCharacterPointer(CBaseEntity *pThisPtr);

#endif
