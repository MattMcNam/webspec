/*
 *  helpers.cpp
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "helpers.h"

int GetClientIndexForUserID(int userid) {
	//Ensure userid is a sane value
	if (userid < 0 || userid > USHRT_MAX)
		return 0;

	IPlayerInfo *playerInfo;
	//Start at 1 to skip Console/Worldspawn
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		playerInfo = playerInfoManager->GetPlayerInfo(engine->PEntityOfEntIndex(i));
		if (!playerInfo || !playerInfo->IsConnected())
			continue;

		if (playerInfo->GetUserID() == userid)
			return i;
	}
	
	return 0;
}
