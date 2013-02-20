/*
 *  definitions.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#ifdef _WIN32
#include <string>
#else
#include <strings.h>
#endif
// Event ints
static const char *eventInts[] = {
	"tournament_stateupdate",
	"player_death",
	"player_spawn",
	0
};

enum TFEvents {
	Event_TournamentState = 0,
	Event_PlayerDeath = 1,
	Event_PlayerSpawn = 2
};

extern int GetEventIntForName(const char *name);

// Packet signatures
// Change these from SourceTV2D's to make more sense
// TODO: Change to binary
#define WSPacket_Init 'I'
#define WSPacket_TeamInfo 'B'
#define WSPacket_PlayerDeath 'K'
#define WSPacket_PlayerSpawn 'S'

#endif
