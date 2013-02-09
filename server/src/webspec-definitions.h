/*
 *  webspec-definitions.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD License
 *  http://opensource.org/licenses/BSD-3-Clause
 *
 */

#ifndef WEBSPEC_DEFINITIONS_H
#define WEBSPEC_DEFINITIONS_H

// Event ints
static const char *eventInts[] = {
	"tournament_stateupdate",
	0
};

static int GetEventIntForName(const char *name) {
	int i = -1;
	while (eventInts[++i]) {
		if (!strcmp(name, eventInts[i])) return i;
	}
	return -1;
}

enum TFEvents {
	Event_TournamentState = 0
};

// Packet signatures
// Change these from SourceTV2D's to make more sense
// Change to binary TODO
#define WSPACKET_Init 'I'
#define WSPACKET_TeamInfo 'B'

#endif