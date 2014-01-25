/*
 *  webspec.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#ifndef WEBSPEC_H
#define WEBSPEC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <algorithm>
#include <vector>
#include <libwebsockets.h>

#ifdef _LINUX
#include <unistd.h>
#endif

#include "callbacks.h"
#include "definitions.h"
#include "helpers.h"
#include "offsets.h"
#include "vfuncs.h"

#include "interface.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "igameevents.h"
#include "eiface.h"
#include "convar.h"
#include "tier2/tier2.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Interfaces from the engine
IVEngineServer *engine = NULL; // helper functions (messaging clients, loading content, making entities, running commands, etc)
IGameEventManager *gameEventManager = NULL; // game events interface
IPlayerInfoManager *playerInfoManager = NULL; // game dll interface to interact with players
IServerGameEnts *serverGameEnts = NULL; // Maybe get player entities for class etc. ??
IServerGameDLL *serverGameDLL = NULL; // Offsets

CGlobalVars *gpGlobals = NULL;

#define WEBSPEC_UPDATE_RATE_IN_SECONDS 0.0625

CUtlVector<struct libwebsocket *> ws_spectators;
string_t ws_teamName[2];
bool ws_teamReadyState[2];
bool ws_shouldListen = false;

//=================================================================================
// Main plugin class
//=================================================================================
class WebSpecPlugin: public IServerPluginCallbacks, public IGameEventListener
{
public:
	WebSpecPlugin();
	~WebSpecPlugin();

	// IServerPluginCallbacks methods
	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory );
	virtual void			Unload( void );
	virtual void			Pause( void );
	virtual void			UnPause( void );
	virtual const char     *GetPluginDescription( void );      
	virtual void			LevelInit( char const *pMapName );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void			GameFrame( bool simulating );
	virtual void			LevelShutdown( void );
	virtual void			ClientActive( edict_t *pEntity );
	virtual void			ClientDisconnect( edict_t *pEntity );
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername );
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict );
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args );
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID );
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue );

	// IGameEventListener Interface
	virtual void FireGameEvent( KeyValues * event );

	// Event Handlers
	void EventHandler_TeamInfo(KeyValues *event);
	void EventHandler_PlayerDeath(KeyValues *event);
	void EventHandler_PlayerSpawn(KeyValues *event);
private:
	struct libwebsocket_context *wsContext;
	int wsPort;
};

void SendPacketToAll(char *buffer, int length);
void SendPacketToOne(char *buffer, int length, struct libwebsocket *wsi);

#endif
