/*
 *  definitions.cpp
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "definitions.h"

int GetEventIntForName(const char *name) {
	int i = -1;
	while (eventInts[++i]) {
		if (!strcmp(name, eventInts[i])) return i;
	}
	return -1;
}
