/*
 *  callbacks.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <stdio.h>
#include <algorithm>
#include <vector>
#include <libwebsockets.h>

#include "definitions.h"
#include "helpers.h"
#include "offsets.h"
#include "vfuncs.h"

#include "convar.h"
#include "eiface.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"

extern IVEngineServer *engine;
extern IPlayerInfoManager *playerInfoManager;
extern IServerGameEnts *serverGameEnts;
extern CGlobalVars *gpGlobals;

extern std::vector<struct libwebsocket *> ws_spectators;
extern string_t ws_teamName[2];

extern void SendPacketToAll(char *buffer, int length);
extern void SendPacketToOne(char *buffer, int length, struct libwebsocket *wsi);

//=================================================================================
// Protocol list for libwebsockets &
// static methods for various libwebsockets callbacks
//=================================================================================

extern int webspec_callback_http(struct libwebsocket_context *ctx, struct libwebsocket *wsi, 
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);

extern int webspec_callback(struct libwebsocket_context *ctx, struct libwebsocket *wsi, 
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len);

static struct libwebsocket_protocols wsProtocols[] = {
	// HTTP required I think, test TODO
	{
		"http-only",
		webspec_callback_http,
		0
	},
	{
		"webspec",
		webspec_callback,
		0
	},
	{ //Also required I think, untested, TODO
		NULL, NULL, 0
	}
};

#endif
