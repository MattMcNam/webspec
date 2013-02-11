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

#endif
