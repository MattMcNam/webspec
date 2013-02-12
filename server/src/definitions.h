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

#include <string>

// Event ints
static const char *eventInts[] = {
	"tournament_stateupdate",
	0
};

enum TFEvents {
	Event_TournamentState = 0
};

extern int GetEventIntForName(const char *name);

// Packet signatures
// Change these from SourceTV2D's to make more sense
// TODO: Change to binary
#define WSPACKET_Init 'I'
#define WSPACKET_TeamInfo 'B'

#endif
