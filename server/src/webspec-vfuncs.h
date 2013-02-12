/*
 *  webspec-vfuncs.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#ifndef WEBSPEC_VFUNCS_H
#define WEBSPEC_VFUNCS_H

class WSEmptyClass {};

class CBaseCombatWeapon;
class CBaseCombatCharacter;

//=================================================================================
// Virtual function indexes
// 
// If anything is likely to break, it's in here, but unfortunately it seems to be
// the only way to do certain things.
// 
// Indexes are obtained via an IDA script provided by AlliedModders. Looking to
// set up server to automatically check indexes after updates.
//  Current CBaseEntity indexes: https://dl.dropbox.com/u/6635591/links/vfunc-indexes-CBaseEntity.txt
//  Current CBaseCombatCharacter indexes: https://dl.dropbox.com/u/6635591/links/vfunc-indexes-CBaseCombatCharacter.txt
//  -Linux indexes, Windows is _usually_ 1 less
//=================================================================================

#ifdef _LINUX
#define VFUNCINDEX_CBaseCombatCharacter_Weapon_GetSlot 267
#define VFUNCINDEX_CBaseEntity_MyCombatCharacterPointer 72
#else
#define VFUNCINDEX_CBaseCombatCharacter_Weapon_GetSlot 266
#define VFUNCINDEX_CBaseEntity_MyCombatCharacterPointer 71
#endif

extern CBaseCombatWeapon *CBaseCombatCharacter_Weapon_GetSlot(CBaseCombatCharacter *pThisPtr, int slot);

extern CBaseCombatCharacter *CBaseEntity_MyCombatCharacterPointer(CBaseEntity *pThisPtr);


//=================================================================================
// Warning, this gets real ugly real fast, skip to bottom to actually setup
// functions for use in plugin
// 
// Mani's macros for calling virtual functions
// May just rewrite as functions, macros are unreadable
//=================================================================================

#ifdef __linux__
#define VFUNC_OS_DEP void *addr;	} u; 	u.addr = func;
#else
#define VFUNC_OS_DEP struct {void *addr; intptr_t adjustor;} s; } u; u.s.addr = func; u.s.adjustor = 0;
#endif

#define VFUNC_SETUP_PTR(_vfunc_index)  \
{ \
	void **this_ptr = *(void ***)&pThisPtr; \
	void **vtable = *(void ***)pThisPtr; \
	void *func = vtable[_vfunc_index]

// Macros for defining functions to call vfuncs via offset.

#define VFUNC_CALL0(_vfunc_index, _return_type, _class_type, _func_name ) \
	_return_type _func_name(_class_type *pThisPtr) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)();} 

#define VFUNC_CALL1(_vfunc_index, _return_type, _class_type, _func_name, _param1) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1);} 

#define VFUNC_CALL2(_vfunc_index, _return_type, _class_type, _func_name, _param1, _param2) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1, _param2); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2);} 

#define VFUNC_CALL3(_vfunc_index, _return_type, _class_type, _func_name, _param1, _param2, _param3) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1, _param2, _param3); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3);} 

#define VFUNC_CALL4(_vfunc_index, _return_type, _class_type, _func_name, _param1, _param2, _param3, _param4) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4);} 

#define VFUNC_CALL5(_vfunc_index, _return_type, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5);} 

#define VFUNC_CALL6(_vfunc_index, _return_type, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5, _param6) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5, _param6 p6) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5, _param6); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5, p6);} 

#define VFUNC_CALL7(_vfunc_index, _return_type, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5, _param6, _param7) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5, _param6 p6, _param7 p7) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5, _param6, _param7); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5, p6, p7);} 

#define VFUNC_CALL8(_vfunc_index, _return_type, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5, _param6, _param7, _param8) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5, _param6 p6, _param7 p7, _param8 p8) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5, _param6, _param7, _param8); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5, p6, p7, p8);} 

#define VFUNC_CALL9(_vfunc_index, _return_type, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5, _param6, _param7, _param8, _param9) \
	_return_type _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5, _param6 p6, _param7 p7, _param8 p8, _param9) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { _return_type (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5, _param6, _param7, _param8, _param9); \
	VFUNC_OS_DEP \
 	return (_return_type) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5, p6, p7, p8, p9);} 

#define VFUNC_CALL0_void(_vfunc_index, _class_type, _func_name ) \
	void _func_name(_class_type *pThisPtr) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)();} 

#define VFUNC_CALL1_void(_vfunc_index, _class_type, _func_name, _param1) \
	void _func_name(_class_type *pThisPtr, _param1 p1) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1);} 

#define VFUNC_CALL2_void(_vfunc_index, _class_type, _func_name, _param1, _param2) \
	void _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1, _param2); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2);} 

#define VFUNC_CALL3_void(_vfunc_index, _class_type, _func_name, _param1, _param2, _param3) \
	void _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1, _param2, _param3); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3);} 

#define VFUNC_CALL4_void(_vfunc_index, _class_type, _func_name, _param1, _param2, _param3, _param4) \
	void _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4);} 

#define VFUNC_CALL5_void(_vfunc_index, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5) \
	void _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5);} 

#define VFUNC_CALL6_void(_vfunc_index, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5, _param6) \
	void _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5, _param6 p6) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5, _param6); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5, p6);} 

#define VFUNC_CALL7_void(_vfunc_index, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5, _param6, _param7) \
	void _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5, _param6 p6, _param7 p7) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5, _param6, _param7); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5, p6, p7);} 

#define VFUNC_CALL8_void(_vfunc_index, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5, _param6, _param7, _param8) \
	void _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5, _param6 p6, _param7 p7, _param8 p8) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5, _param6, _param7, _param8); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5, p6, p7, p8);} 

#define VFUNC_CALL9_void(_vfunc_index, _class_type, _func_name, _param1, _param2, _param3, _param4, _param5, _param6, _param7, _param8, _param9) \
	void _func_name(_class_type *pThisPtr, _param1 p1, _param2 p2, _param3 p3, _param4 p4, _param5 p5, _param6 p6, _param7 p7, _param8 p8, _param9) \
	VFUNC_SETUP_PTR(_vfunc_index); \
	union { void (WSEmptyClass::*mfpnew)(_param1, _param2, _param3, _param4, _param5, _param6, _param7, _param8, _param9); \
	VFUNC_OS_DEP \
 	(void) (reinterpret_cast<WSEmptyClass*>(this_ptr)->*u.mfpnew)(p1, p2, p3, p4, p5, p6, p7, p8, p9);} 

VFUNC_CALL1(VFUNCINDEX_CBaseCombatCharacter_Weapon_GetSlot, CBaseCombatWeapon *, CBaseCombatCharacter, CBaseCombatCharacter_Weapon_GetSlot, int);

VFUNC_CALL0(VFUNCINDEX_CBaseEntity_MyCombatCharacterPointer, CBaseCombatCharacter *, CBaseEntity, CBaseEntity_MyCombatCharacterPointer);

#endif
