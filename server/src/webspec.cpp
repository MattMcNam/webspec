/*
 *  webspec.cpp
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD License
 *  http://opensource.org/licenses/BSD-3-Clause
 *
 */

#include "webspec.h"
#include "webspec-definitions.h"
#include "webspec-helpers.h"
#include "webspec-dumping.h"

// Global vars
string_t ws_teamName[2];
bool ws_teamReadyState[2];
std::vector<struct libwebsocket *> ws_spectators;
bool ws_shouldListen = false;

//Offsets
int ws_offset_CTFPlayer_iClass;
int ws_offset_CBaseObject_iMaxHealth;

//=================================================================================
// Callback from thread, only managing connections for now
// May be expanded to handle caster->players chat during pauses,
//   depending on if the feature is allowed, or even wanted
//=================================================================================

static int webspec_callback_http(struct libwebsocket_context *ctx, struct libwebsocket *wsi, 
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
	return 0;
}

static int webspec_callback(struct libwebsocket_context *ctx, struct libwebsocket *wsi, 
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason) {
		case LWS_CALLBACK_ESTABLISHED:
		{
			// New connection
			ws_spectators.push_back(wsi);
			
			// Send basic game info to let client set up
			// MapName, Server name (may remove), current team names (TF2 5-letter, not full)
			char *buffer = (char*)malloc(256);
			char *mapName = (char*) STRING(gpGlobals->mapname);
			ConVarRef hostNameCVar = ConVarRef("hostname");
			char *hostname;
			if (hostNameCVar.IsValid())
				hostname = (char *)hostNameCVar.GetString();
			else
				hostname = "WebSpec Demo Server"; //Can't imagine when hostname would be invalid, but this is Source
			int length = sprintf(buffer, "%c%s:%s:%s:%s", WSPACKET_Init, mapName, hostname, STRING(ws_teamName[1]), STRING(ws_teamName[0]));

			SendPacketToOne(buffer, length, wsi);
			free(buffer);

			//Send connected players
			IPlayerInfo *playerInfo;
			for (int i=1; i<=gpGlobals->maxClients; i++) {
				playerInfo = playerInfoManager->GetPlayerInfo(engine->PEntityOfEntIndex(i));
				if (playerInfo != NULL && playerInfo->IsConnected()) {
					buffer = (char *)malloc(256);
					int userid = playerInfo->GetUserID();
					int teamid = playerInfo->GetTeamIndex();
					int health = playerInfo->GetHealth();
					int maxHealth = playerInfo->GetMaxHealth();
					bool alive = !playerInfo->IsDead();
					string_t playerName = MAKE_STRING(playerInfo->GetName());
						
					CBaseEntity *playerEntity = serverGameEnts->EdictToBaseEntity(engine->PEntityOfEntIndex(i));
					int playerClass =  *MakePtr(int*, playerEntity, ws_offset_CTFPlayer_iClass);

					int length = sprintf(buffer, "%c%d:%d:%d:%d:%d:%d:0:0:%s", 'C', userid, teamid, playerClass,
						health, maxHealth, alive, STRING(playerName));

					SendPacketToOne(buffer, length, wsi);
					free(buffer);
				}
			}

			break;
		}
		case LWS_CALLBACK_RECEIVE:
		{
			//Echo back whatever is received with a . preceding
			//Useful to quickly test if WebSockets is working correctly

			//Create buffer for response, + LibWebsockets padding (don't worry about this)
			//len + 1 for '.'
			unsigned char *buffer = (unsigned char *) malloc(len + 1 + 
				LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING);

			//Put initial . into buffer, after padding
			buffer[LWS_SEND_BUFFER_PRE_PADDING] = '.';

			//Put sent message into response
			for (unsigned int i=0; i < len; i++) {
				buffer[LWS_SEND_BUFFER_PRE_PADDING + 1 + i] = ((char *) in)[i];
			}

			//Actually send
			libwebsocket_write(wsi, &buffer[LWS_SEND_BUFFER_PRE_PADDING], len+1, LWS_WRITE_TEXT);

			//Free memory
			free(buffer);
			break;
		}
		case LWS_CALLBACK_CLOSED:
		{
			std::vector<struct libwebsocket *>::iterator it;
			it = std::find(ws_spectators.begin(), ws_spectators.end(), wsi);
			ws_spectators.erase(it);
		}
		default:
			break;
	}

	return 0;
}

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

//=================================================================================
// Thread to listen to clients without blocking
// Based heavily on https://developer.valvesoftware.com/wiki/Threads
//=================================================================================
struct WSServerThreadParams_t {
	libwebsocket_context *ctx;
};

static unsigned WSServerThread(void *params) {
	WSServerThreadParams_t *vars = (WSServerThreadParams_t *)params;
	
	while (ws_shouldListen) {
		libwebsocket_service(vars->ctx, 50); //check for events every 50ms
	}

	delete vars;
	return 0;
}
// 
// The plugin is a static singleton that is exported as an interface
//
WebSpecPlugin g_WebSpecPlugin;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(WebSpecPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_WebSpecPlugin );

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
WebSpecPlugin::WebSpecPlugin()
{
	wsPort = 28020;
}

WebSpecPlugin::~WebSpecPlugin()
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool WebSpecPlugin::Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	ConnectTier1Libraries( &interfaceFactory, 1 );
	ConnectTier2Libraries( &interfaceFactory, 1 );

	playerInfoManager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER,NULL);
	if ( !playerInfoManager )
	{
		Warning( "[WebSpec] Unable to load PlayerInfoManager!\n" );
		return false;
	}

	gameEventManager = (IGameEventManager *)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER,NULL);
	if(	!gameEventManager )
	{
		Warning( "[WebSpec] Unable to load GameEventManager!\n" );
		return false;
	}

	engine = (IVEngineServer *)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	if (!engine) {
		Warning("[WebSpec] Unable to load VEngineServer!\n");
		return false;
	}

	serverGameEnts = (IServerGameEnts *)gameServerFactory(INTERFACEVERSION_SERVERGAMEENTS, NULL);
	if (!serverGameEnts) {
		Warning("[WebSpec] Unable to load ServerGameEnts!\n");
		return false;
	}

	serverGameDLL = (IServerGameDLL *)gameServerFactory("ServerGameDLL008", NULL); // ServerGameDLL008 for TF2, not in hl2sdk-ob-valve
	if (!serverGameDLL) {
		return false;
	}

	gameEventManager->AddListener(this, true);

	//Init global variables
	gpGlobals = playerInfoManager->GetGlobalVars();
	ws_teamName[0] = MAKE_STRING("BLU");
	ws_teamName[1] = MAKE_STRING("RED");
	ws_teamReadyState[0] = false;
	ws_teamReadyState[1] = false;

	//Get offsets
	ws_offset_CTFPlayer_iClass = WS_FindOffset("CTFPlayer", "m_iClass");
	ws_offset_CBaseObject_iMaxHealth = WS_FindOffset("CBaseObject", "m_iMaxHealth");

	//Init WebSocket server
	const char *wsInterface = NULL;
	const char *cert_path = NULL; //No SSL
	const char *key_path = NULL;
	int options = 0; //No special options needed

	wsContext = libwebsocket_create_context(wsPort, wsInterface, wsProtocols,
		libwebsocket_internal_extensions, cert_path, key_path, NULL, -1, -1, options, NULL);

	if (wsContext == NULL)
		Msg("[WebSpec] failed to init libwebsockets\n");

	//Start WebSpec server
	ws_shouldListen = true;
	WSServerThreadParams_t *params = new WSServerThreadParams_t;
	params->ctx = wsContext;
	CreateSimpleThread( WSServerThread, params );

	//Register cvars
	ConVar_Register( 0 );
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void WebSpecPlugin::Unload( void )
{
	gameEventManager->RemoveListener( this ); // make sure we are unloaded from the event system

	ws_shouldListen = false;
	Sleep(60);
	libwebsocket_context_destroy(wsContext);
	//Context will be cleaned up in thread, thread should then end

	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is paused (i.e should stop running but isn't unloaded)
//---------------------------------------------------------------------------------
void WebSpecPlugin::Pause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unpaused (i.e should start executing again)
//---------------------------------------------------------------------------------
void WebSpecPlugin::UnPause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *WebSpecPlugin::GetPluginDescription( void )
{
	return "WebSpec Plugin";
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void WebSpecPlugin::LevelInit( char const *pMapName )
{
	//WSOnNewMap( pMapName );
	gameEventManager->AddListener( this, true );
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void WebSpecPlugin::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void WebSpecPlugin::GameFrame( bool simulating )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void WebSpecPlugin::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	gameEventManager->RemoveListener( this );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void WebSpecPlugin::ClientActive( edict_t *pEntity )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void WebSpecPlugin::ClientDisconnect( edict_t *pEntity )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on 
//---------------------------------------------------------------------------------
void WebSpecPlugin::ClientPutInServer( edict_t *pEntity, char const *playername )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void WebSpecPlugin::SetCommandClient( int index )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void WebSpecPlugin::ClientSettingsChanged( edict_t *pEdict )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT WebSpecPlugin::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT WebSpecPlugin::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT WebSpecPlugin::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a cvar value query is finished
//---------------------------------------------------------------------------------
void WebSpecPlugin::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------
void WebSpecPlugin::FireGameEvent( KeyValues * event )
{
	const char *name = event->GetName();
	int eventInt = GetEventIntForName(name);

	switch (eventInt) {
	case Event_TournamentState:
	{
		EventHandler_TeamInfo(event);
		break;
	}
	default:
		break;
	}
}

//=================================================================================
// Various Event Handlers
//=================================================================================

void WebSpecPlugin::EventHandler_TeamInfo(KeyValues *event) {
	int userID = event->GetInt("userid");
	int clientIndex = WS_GetClientOfUserID(userID);
	int teamID = playerInfoManager->GetPlayerInfo(engine->PEntityOfEntIndex(clientIndex))->GetTeamIndex();
	bool nameChanged = (event->GetInt("namechange") > 0);
	
	string_t teamName;
	int readyState;
	char *buffer = (char*)malloc(30);
	
	if (teamID == TFTeam_Red) {
		if (nameChanged)
			ws_teamName[1] = MAKE_STRING(event->GetString("newname"));
		else
			ws_teamReadyState[1] = (event->GetInt("readystate") > 0);

		teamName = ws_teamName[1];
		readyState = ws_teamReadyState[1];
	} else {
		if (nameChanged)
			ws_teamName[0] = MAKE_STRING(event->GetString("newname"));
		else
			ws_teamReadyState[0] = (event->GetInt("readystate") > 0);

		teamName = ws_teamName[0];
		readyState = ws_teamReadyState[0];
	}

	int length = sprintf(buffer, "%c%i:%s:%i", WSPACKET_TeamInfo, teamID, teamName, readyState);
	SendPacketToAll(buffer, length);

	free(buffer);
}

static void SendPacketToAll(char *buffer, int length) {
	unsigned char *packetBuffer = (unsigned char*)malloc(length + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING);

	//Put sent message into response
	for (int i=0; i < length; i++) {
		packetBuffer[LWS_SEND_BUFFER_PRE_PADDING + i] = buffer[i];
	}
	
	//Send to all clients
	struct libwebsocket *wsi;
	for (unsigned int i=0; i<ws_spectators.size(); i++) {
		wsi = ws_spectators.at(i);
		libwebsocket_write(wsi, &packetBuffer[LWS_SEND_BUFFER_PRE_PADDING], length, LWS_WRITE_TEXT);
	}
	free(packetBuffer);
}

static void SendPacketToOne(char *buffer, int length, struct libwebsocket *wsi) {
	unsigned char *packetBuffer = (unsigned char *)malloc(length + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING);

	//Put sent message into response
	for (int i=0; i < length; i++) {
		packetBuffer[LWS_SEND_BUFFER_PRE_PADDING + i] = buffer[i];
	}
	
	//Send to given client
	libwebsocket_write(wsi, &packetBuffer[LWS_SEND_BUFFER_PRE_PADDING], length, LWS_WRITE_TEXT);
	free(packetBuffer);
}