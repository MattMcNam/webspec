/*
 *  vfuncs.cpp
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "vfuncs.h"

CBaseCombatWeapon *CBaseCombatCharacter_Weapon_GetSlot(CBaseCombatCharacter *pThisPtr, int slot) {
	void **this_ptr = *(void ***)&pThisPtr;
	void **vtable = *(void ***)pThisPtr;
	void *func = vtable[VFUNCINDEX_CBaseCombatCharacter_Weapon_GetSlot];

	union { CBaseCombatWeapon *(WSEmptyClass::*mfpnew)(int);
#if !defined(_LINUX) && !defined(_OSX)
		void *addr;	} u; 	u.addr = func;
#else // GCC's member function pointers all contain a this pointer adjustor. You'd probably set it to 0 
		struct {void *addr; intptr_t adjustor;} s; } u; u.s.addr = func; u.s.adjustor = 0;
#endif
	return (CBaseCombatWeapon *) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(slot);
}

CBaseCombatCharacter *CBaseEntity_MyCombatCharacterPointer(CBaseEntity *pThisPtr) {
	void **this_ptr = *(void ***)&pThisPtr;
	void **vtable = *(void ***)pThisPtr;
	void *func = vtable[VFUNCINDEX_CBaseEntity_MyCombatCharacterPointer];

	union { CBaseCombatCharacter *(WSEmptyClass::*mfpnew)();
#if !defined(_LINUX) && !defined(_OSX)
		void *addr;	} u; 	u.addr = func;
#else // GCC's member function pointers all contain a this pointer adjustor. You'd probably set it to 0 
		struct {void *addr; intptr_t adjustor;} s; } u; u.s.addr = func; u.s.adjustor = 0;
#endif
	return (CBaseCombatCharacter *) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)();
}

QAngle& CBaseEntity_EyeAngles(CBaseEntity *pThisPtr) {
	void **this_ptr = *(void ***)&pThisPtr;
	void **vtable = *(void ***)pThisPtr;
	void *func = vtable[VFUNCINDEX_CBaseEntity_EyeAngles];

	union { QAngle& (WSEmptyClass::*mfpnew)();
#if !defined(_LINUX) && !defined(_OSX)
		void *addr;	} u; 	u.addr = func;
#else // GCC's member function pointers all contain a this pointer adjustor. You'd probably set it to 0 
		struct {void *addr; intptr_t adjustor;} s; } u; u.s.addr = func; u.s.adjustor = 0;
#endif
	return (QAngle&) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)();
}
