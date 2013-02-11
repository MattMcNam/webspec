/*
 *  webspec-helpers.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD License
 *  http://opensource.org/licenses/BSD-3-Clause
 *
 */

#ifndef WEBSPEC_HELPERS_H
#define WEBSPEC_HELPERS_H

#include "webspec.h"
#include "server_class.h"

enum TFTeam {
	TFTeam_Unassigned = 0,
	TFTeam_Spectator = 1,
	TFTeam_Red = 2,
	TFTeam_Blue = 3
};

enum TFClass {
	TFClass_Unknown = 0,
	TFClass_Scout,
	TFClass_Sniper,
	TFClass_Soldier,
	TFClass_DemoMan,
	TFClass_Medic,
	TFClass_Heavy,
	TFClass_Pyro,
	TFClass_Spy,
	TFClass_Engineer
};

#define WSCompileRoundFloat(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

static int WS_GetClientOfUserID(int userid) {
	if (userid < 0 || userid > USHRT_MAX)
		return 0;

	IPlayerInfo *playerInfo;
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		playerInfo = playerInfoManager->GetPlayerInfo(engine->PEntityOfEntIndex(i));
		if (!playerInfo || !playerInfo->IsConnected())
			continue;

		if (playerInfo->GetUserID() == userid)
			return i;
	}

	return 0;
}

//=================================================================================
// MakePtr, CrawlForPropOffset and WS_FindOffset are based on code by various
// posters from AlliedMods and GameDeception.
// They have been modified to be able to find any property that is at any point in
// the hierarchy of a class.
//   eg "m_iHealth" will get CTFPlayer->m_iHealth, 
//   and "m_iClass" will get CTFPlayer->m_PlayerClass->m_iClass
// 
// TODO, specify subclass if two classes have the same named property, to find
// correct one
//=================================================================================

#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))

static bool CrawlForPropOffset(SendTable *sTable, const char *propName, int &offset) {
	for (int i=0; i < sTable->GetNumProps(); i++) {
		SendProp *sProp = sTable->GetProp(i);
		if (strcmp(sProp->GetName(),"000") == 0) //End of an array
			continue;

		SendTable *sChildTable = sProp->GetDataTable();

		//Check if it is an array, don't care for these atm so skip them
		bool isArray = false;
		if (sChildTable && sChildTable->GetNumProps() > 0) {
			if (   !strcmp(sChildTable->GetProp(0)->GetName(), "000")
				|| !strcmp(sChildTable->GetProp(0)->GetName(), "lengthproxy"))
				isArray = true;
		}

		if (!isArray) {
			//If we have our property, add to the offset and start returning
			if (strcmp(sProp->GetName(), propName) == 0) {
				offset += sProp->GetOffset();
				return true;
			}
			
			//If we find a subtable, search it for the property, 
			//but keep current offset in case it isn't found here
			if (sProp->GetType() == DPT_DataTable) {
				int origOffset = offset;
				offset += sProp->GetOffset();
				bool found = CrawlForPropOffset(sChildTable, propName, offset);
				if (found) {
					return true;
				} else {
					offset = origOffset;
				}
			}
		} else {
			continue;
		}

		if (strcmp(sProp->GetName(), "000") == 0) //More array stuff from dumping function, may not be needed here
			break;
	}
	return 0;
}

static int WS_FindOffset(const char *className, const char *propName) {
	ServerClass *sc = serverGameDLL->GetAllServerClasses();
	while (sc) {
		if (Q_strcmp(sc->GetName(), className) == 0) {
			SendTable *sTable = sc->m_pTable;
			if (sTable) {
				int offset = 0;
				bool found = CrawlForPropOffset(sTable, propName, offset);
				if (!found)
					offset = 0;
				return offset;
			}
		}
		sc = sc->m_pNext;
	}
	return 0;
}

#endif
