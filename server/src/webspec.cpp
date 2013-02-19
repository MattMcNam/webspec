/*
 *  webspec.cpp
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "webspec.h"

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

float g_lastUpdateTime;

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
	
	WSOffsets::PrepareOffsets();

	//Init WebSocket server
	lws_context_creation_info *lwsInfo = new lws_context_creation_info();
	lwsInfo->port = wsPort;
	lwsInfo->iface = NULL;
	lwsInfo->protocols = wsProtocols;
	lwsInfo->extensions = libwebsocket_get_internal_extensions();
	lwsInfo->ssl_cert_filepath = NULL;
	lwsInfo->ssl_private_key_filepath = NULL;
	lwsInfo->ssl_ca_filepath = NULL;
	lwsInfo->gid = -1;
	lwsInfo->uid = -1;
	lwsInfo->options = 0;
	lwsInfo->user = NULL;
	lwsInfo->ka_time = 0;

	wsContext = libwebsocket_create_context(lwsInfo);

	if (wsContext == NULL)
		Msg("[WebSpec] failed to init libwebsockets\n");

	//Start WebSpec server
	ws_shouldListen = true;
	WSServerThreadParams_t *params = new WSServerThreadParams_t;
	params->ctx = wsContext;
	CreateSimpleThread( WSServerThread, params );

	g_lastUpdateTime = gpGlobals->curtime;

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
#ifdef _LINUX
	usleep(60*1000);
#else
	Sleep(60);
#endif
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
	if (gpGlobals->curtime - g_lastUpdateTime > WEBSPEC_UPDATE_RATE_IN_SECONDS) {
		
		char *buffer = (char *)malloc(2048);
		Vector playerOrigin;
		QAngle playerAngles;
		float playerUberCharge;
		int userid, health, playerClass;

		IPlayerInfo *playerInfo;
		CBaseEntity *playerEntity;
		int bufferLength;

		bufferLength = sprintf(buffer, "O");

		for (int i = 1; i < gpGlobals->maxClients; i++) {
			playerInfo = playerInfoManager->GetPlayerInfo(engine->PEntityOfEntIndex(i));
			if (playerInfo == NULL || !playerInfo->IsConnected()) continue;

			if (strlen(buffer) > 1)
				sprintf(buffer, "%s|", buffer);

			userid = playerInfo->GetUserID();
			health = playerInfo->GetHealth();
			playerOrigin = playerInfo->GetAbsOrigin();
			playerAngles = playerInfo->GetAbsAngles();

			playerEntity = serverGameEnts->EdictToBaseEntity(engine->PEntityOfEntIndex(i));
			playerClass = *MakePtr(int*, playerEntity, WSOffsets::pCTFPlayer__m_iClass);

			if (playerClass == TFClass_Medic) {
				CBaseCombatCharacter *playerCombatCharacter = CBaseEntity_MyCombatCharacterPointer(playerEntity);
				CBaseCombatWeapon *slot1Weapon = CBaseCombatCharacter_Weapon_GetSlot(playerCombatCharacter, 1);
				
				playerUberCharge = *MakePtr(float*, slot1Weapon, WSOffsets::pCWeaponMedigun__m_flChargeLevel);
			} else {
				playerUberCharge = 0.0f;
			}

			bufferLength = sprintf(buffer, "%s%d:%d:%d:%d:%d:%d", buffer, userid, 
				Round(playerOrigin.x), Round(playerOrigin.y), Round(playerAngles.y),
				health, Round(playerUberCharge));
		}

		if (bufferLength == 1) return;

		SendPacketToAll(buffer, bufferLength);

		g_lastUpdateTime = gpGlobals->curtime;
	}
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
	int clientIndex = GetClientIndexForUserID(userID);
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

	int length = sprintf(buffer, "%c%i:%s:%i", WSPACKET_TeamInfo, teamID, STRING(teamName), readyState);
	SendPacketToAll(buffer, length);

	free(buffer);
}

void SendPacketToAll(char *buffer, int length) {
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

void SendPacketToOne(char *buffer, int length, struct libwebsocket *wsi) {
	unsigned char *packetBuffer = (unsigned char *)malloc(length + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING);

	//Put sent message into response
	for (int i=0; i < length; i++) {
		packetBuffer[LWS_SEND_BUFFER_PRE_PADDING + i] = buffer[i];
	}
	
	//Send to given client
	libwebsocket_write(wsi, &packetBuffer[LWS_SEND_BUFFER_PRE_PADDING], length, LWS_WRITE_TEXT);
	free(packetBuffer);
}
