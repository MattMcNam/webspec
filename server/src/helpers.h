/*
 *  helpers.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#ifndef HELPERS_H
#define HELPERS_H

#ifdef WIN32
#include "win32_helpers.h"
#endif

#include "eiface.h"
#include "game/server/iplayerinfo.h"

extern IVEngineServer *engine;
extern IPlayerInfoManager *playerInfoManager;
extern CGlobalVars *gpGlobals;

enum TFTeam {
	TFTeam_Unassigned = 0,
	TFTeam_Spectator = 1,
	TFTeam_Red = 2,
	TFTeam_Blue = 3
};

// Ordered by TF2's internal indexes, 
// not class selection screen ingame
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

#define Round(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))

extern int GetClientIndexForUserID(int userid);

#endif
