/*
 *  webspec_plugin.cpp
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD License
 *  http://opensource.org/licenses/BSD-3-Clause
 *
 */

#include "webspec.h"
#include "webspec-packetCodes.h"

// Global vars
string_t ws_teamNameBlu = MAKE_STRING("BLU");
string_t ws_teamNameRed = MAKE_STRING("RED");

//=================================================================================
// Callback from thread, only managing connections for now
// May be expanded to handle caster->players chat during pauses,
//   depending on if the feature is allowed, or even wanted
//=================================================================================

static bool wsShouldListen = false;

static int webspec_callback_http(struct libwebsocket_context *ctx, struct libwebsocket *wsi, 
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
	return 0;
}

static int webspec_callback(struct libwebsocket_context *ctx, struct libwebsocket *wsi, 
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason) {
		case LWS_CALLBACK_ESTABLISHED: { // New connection
			Msg("[WebSpec] New connection\n");
			
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

			int length = sprintf(buffer, "%s%s:%s:%s:%s", WSPACKET_INIT, mapName, hostname, STRING(ws_teamNameRed), STRING(ws_teamNameBlu));

			unsigned char *packetBuffer = (unsigned char*)malloc(length + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING);
			//Put sent message into response
			for (int i=0; i < length; i++) {
				packetBuffer[LWS_SEND_BUFFER_PRE_PADDING + i] = buffer[i];
			}

			libwebsocket_write(wsi, &packetBuffer[LWS_SEND_BUFFER_PRE_PADDING], length, LWS_WRITE_TEXT);

			free(packetBuffer);
			free(buffer);

			break;
		}
		case LWS_CALLBACK_RECEIVE: {
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

			//Print to console for debugging
			Msg("[WebSpec] Sending message: %.*s\n", len+1, buffer + LWS_SEND_BUFFER_PRE_PADDING);

			//Actually send
			libwebsocket_write(wsi, &buffer[LWS_SEND_BUFFER_PRE_PADDING], len+1, LWS_WRITE_TEXT);

			//Free memory
			free(buffer);
			break;
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
	
	while (wsShouldListen) {
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
	}

	if ( playerInfoManager )
	{
		gpGlobals = playerInfoManager->GetGlobalVars();
	}

	//Init WebSocket server
	const char *wsInterface = NULL;
	const char *cert_path = NULL; //No SSL
	const char *key_path = NULL;
	int options = 0; //No special options needed

	wsContext = libwebsocket_create_context(wsPort, wsInterface, wsProtocols,
		libwebsocket_internal_extensions, cert_path, key_path, NULL, -1, -1, options, NULL);

	if (wsContext == NULL)
		Msg("[WebSpec] failed to init libwebsockets");

	//Start WebSpec server
	wsShouldListen = true;
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

	wsShouldListen = false;
	Sleep(60);
	libwebsocket_context_destroy(wsContext);
	//Context will be cleaned up in thread, thread should then end

	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	//Msg("here4\n");
	DisconnectTier1Libraries( );
	//Msg("here5\n");
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
	//const char * name = event->GetName();
	//Msg( "WebSpecPlugin::FireGameEvent: Got event \"%s\"\n", name );
}