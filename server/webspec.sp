/*  
 *  webspec.sp
 *  WebSpec
 *  
 *  Heavily based on SourceTV2D, pending rewrite as VSP
 *  
 */

/**
 * Socket control characters
 * Based on srctv2d codes
 * 
 * A: Amount of Web Spectators
 * B: Team info changed
 * C: Player connected
 * D: Player disconnected
 * E: Round ended
 * F: 
 * G: Game chat
 * H: 
 * I: Initial connect from spectator, send gameinfo
 * J: 
 * K: Player killed
 * L: 
 * M: Map change
 * N: Player name change
 * O: Player position change
 * P: Pause
 * Q: 
 * R: Round start
 * S: Player (re)spawn
 * T: Player changed team
 * U: Uber used
 * V: cVar update
 * W: 
 * X: Connection lost
 * Y: Connection regained, send updated gameinfo
 * Z: 
 *
 */
 
#pragma semicolon 1
#include <sourcemod>
#include <sdktools>
#include <websocket> //http://forums.alliedmods.net/showthread.php?t=182615
#include <tf2>
#include <tf2_stocks>

#define PLUGIN_VERSION "0.1"
#define POSITION_UPDATE_RATE 0.3

new WebsocketHandle:g_serverSocket = INVALID_WEBSOCKET_HANDLE;
new Handle:g_childrenWebsocketHandles;
new Handle:g_childrenWebsocketIPs;
new Handle:g_hUpdatePositions = INVALID_HANDLE;

new Handle:g_hostname;

new String:g_teamNameBlu[6], String:g_teamNameRed[6];
new g_teamReadyState[2];

new g_offset_m_iMaxHealth;

public Plugin:myinfo = {
	name = "WebSpec",
	author = "Matthew \"Blue\" McNamara",
	description = "WebSpec server",
	version = PLUGIN_VERSION,
	url = "http://www.mattmcn.com/"
}

public OnPluginStart() {
	g_childrenWebsocketHandles = CreateArray();
	g_childrenWebsocketIPs = CreateArray(ByteCountToCells(33));
	
	AddCommandListener(CmdListener_Say, "say");
	g_hostname = FindConVar("hostname");
	
	g_teamNameRed = "RED";
	g_teamNameBlu = "BLU";
	g_teamReadyState[0] = 0;
	g_teamReadyState[1] = 0;
	
	g_offset_m_iMaxHealth = FindSendPropOffs("CBaseObject", "m_iMaxHealth");
	
	//Hook Source events
	HookEvent("player_death", EventHandler_PlayerDeath);
	HookEvent("player_team", EventHandler_PlayerTeam);
	HookEvent("player_changename", EventHandler_PlayerName);
	
	//Hook TF2 specific events
	HookEvent("tournament_stateupdate", EventHandler_TournamentStateUpdate);
	HookEventEx("player_spawn", EventHandler_PlayerSpawn);
	HookEventEx("post_inventory_application", Event_PlayerInventory); //Supply cabinet
}

public OnAllPluginsLoaded() {
	PrintToServer("AllPlugins");
	decl String:serverIP[40];
	new longip = GetConVarInt(FindConVar("hostip")), pieces[4];
	pieces[0] = (longip >> 24) & 0x000000FF;
	pieces[1] = (longip >> 16) & 0x000000FF;
	pieces[2] = (longip >> 8) & 0x000000FF;
	pieces[3] = longip & 0x000000FF;
	FormatEx(serverIP, sizeof(serverIP), "%d.%d.%d.%d", pieces[0], pieces[1], pieces[2], pieces[3]);
	PrintToServer(serverIP);
	if(g_serverSocket == INVALID_WEBSOCKET_HANDLE)
		g_serverSocket = Websocket_Open(serverIP, 28020, OnWebsocketIncoming, OnWebsocketMasterError, OnWebsocketMasterClose);
}

public OnClientPutInServer(client)
{
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	if(iSize == 0)
		return;
	
	decl String:packetBuffer[128];
	GetClientIP(client, packetBuffer, sizeof(packetBuffer));
	Format(packetBuffer, sizeof(packetBuffer), "C%d:%d:%d:%d:%d:%d:%d:%d:%N", GetClientUserId(client), GetClientTeam(client),
		TF2_GetPlayerClass(client), GetClientHealth(client), GetEntData(client, g_offset_m_iMaxHealth, 4), IsPlayerAlive(client),
		0 /* ubered */, 0 /*uberpercentage*/, client);
	PrintToServer(packetBuffer);
	
	SendToAllChildren(packetBuffer);
}

public OnClientDisconnect(client)
{
	if(IsClientInGame(client))
	{
		new iSize = GetArraySize(g_childrenWebsocketHandles);
		if(iSize == 0)
			return;
		
		decl String:packetBuffer[20];
		Format(packetBuffer, sizeof(packetBuffer), "D%d", GetClientUserId(client));
		
		SendToAllChildren(packetBuffer);
	}
}

public OnPluginEnd() {
	if(g_serverSocket != INVALID_WEBSOCKET_HANDLE)
		Websocket_Close(g_serverSocket);
}

public Action:CmdListener_Say(client, const String:command[], argc)
{
	decl String:packetBuffer[128];
	GetCmdArgString(packetBuffer, sizeof(packetBuffer));
	
	StripQuotes(packetBuffer);
	if(strlen(packetBuffer) == 0)
		return Plugin_Continue;
	
	// Send console messages
	new userid = 0;
	if(client)
		userid = GetClientUserId(client);
	
	Format(packetBuffer, sizeof(packetBuffer), "G%d:%s", userid, packetBuffer);
	
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	for(new i=0;i<iSize;i++)
		Websocket_Send(GetArrayCell(g_childrenWebsocketHandles, i), SendType_Text, packetBuffer);
	
	return Plugin_Continue;
}

public EventHandler_TournamentStateUpdate(Handle:event, const String:name[], bool:dontBroadcast) {
	decl String:teamName[6], String:packetBuffer[11], readyState;
	new userID = GetEventInt(event, "userid");
	new teamID = GetClientTeam(userID);
	new bool:nameChanged = GetEventBool(event, "namechange");
	
	if (teamID == _:TFTeam_Red) {
		if (nameChanged) 
			GetEventString(event, "newname", g_teamNameRed, sizeof(g_teamNameRed));
		else
			g_teamReadyState[0] = GetEventInt(event, "readystate");
		teamName = g_teamNameRed;
		readyState = g_teamReadyState[0];
	} else {
		if (nameChanged) 
			GetEventString(event, "newname", g_teamNameBlu, sizeof(g_teamNameBlu));
		else 
			g_teamReadyState[1] = GetEventInt(event, "readystate");
		teamName = g_teamNameBlu;
		readyState = g_teamReadyState[1];
	}
	
	Format(packetBuffer, sizeof(packetBuffer), "B%d:%s:%d", teamID, teamName, readyState);
	SendToAllChildren(packetBuffer);
}

public EventHandler_PlayerDeath(Handle:event, const String:name[], bool:dontBroadcast)
{
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	if(iSize == 0)
		return;
	
	new victim = GetEventInt(event, "userid");
	new attacker = GetEventInt(event, "attacker");
	
	new String:sBuffer[64];
	GetEventString(event, "weapon", sBuffer, sizeof(sBuffer));
	Format(sBuffer, sizeof(sBuffer), "K%d:%d:%s", victim, attacker, sBuffer);
	
	SendToAllChildren(sBuffer);
}

public EventHandler_PlayerSpawn(Handle:event, const String:name[], bool:dontBroadcast)
{
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	if(iSize == 0)
		return;
	
	PrintToServer("called");
	
	new userid = GetEventInt(event, "userid");
	new tfclass = GetEventInt(event, "class");
	new client = GetClientOfUserId(userid);
	
	decl String:sBuffer[20];
	Format(sBuffer, sizeof(sBuffer), "S%d:%d:%d:%d", userid, tfclass, GetClientHealth(client), GetEntData(client, g_offset_m_iMaxHealth, 4));
	PrintToServer(sBuffer);
	SendToAllChildren(sBuffer);
}

public Event_PlayerInventory(Handle:event, const String:name[], bool:dontBroadcast)
{
	// fired when touching supply cabinet
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	if(iSize == 0)
		return;
	
	PrintToServer("called");
	
	new userid = GetEventInt(event, "userid");
	new client = GetClientOfUserId(userid);
	//class id of 0 = keep current value in client
	
	decl String:sBuffer[20];
	Format(sBuffer, sizeof(sBuffer), "S%d:%d:%d:%d", userid, 0, GetClientHealth(client), GetEntData(client, g_offset_m_iMaxHealth, 4));
	PrintToServer(sBuffer);
	SendToAllChildren(sBuffer);
}

public EventHandler_PlayerName(Handle:event, const String:name[], bool:dontBroadcast)
{
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	if(iSize == 0)
		return;
	
	new userid = GetEventInt(event, "userid");
	decl String:sOldName[MAX_NAME_LENGTH];
	decl String:sNewName[MAX_NAME_LENGTH];
	GetEventString(event, "oldname", sOldName, sizeof(sOldName));
	GetEventString(event, "newname", sNewName, sizeof(sNewName));
	
	if(StrEqual(sNewName, sOldName))
		return;
	
	decl String:sBuffer[MAX_NAME_LENGTH+10];
	Format(sBuffer, sizeof(sBuffer), "N%d:%s", userid, sNewName);
	
	SendToAllChildren(sBuffer);
}

public EventHandler_PlayerTeam(Handle:event, const String:name[], bool:dontBroadcast)
{
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	if(iSize == 0)
		return;
	
	new userid = GetEventInt(event, "userid");
	new team = GetEventInt(event, "team");
	
	if(team == 0)
		return;
	
	decl String:sBuffer[10];
	Format(sBuffer, sizeof(sBuffer), "T%d:%d", userid, team);
	
	SendToAllChildren(sBuffer);
}

public Action:OnWebsocketIncoming(WebsocketHandle:websocket, WebsocketHandle:newWebsocket, const String:remoteIP[], remotePort, String:protocols[256]) {
	PrintToServer("[WS] New connection");
	Websocket_HookChild(newWebsocket, OnWebsocketReceive, OnWebsocketDisconnect, OnChildWebsocketError);
	Websocket_HookReadyStateChange(newWebsocket, OnWebsocketReadyStateChanged);
	
	PushArrayCell(g_childrenWebsocketHandles, newWebsocket);
	PushArrayString(g_childrenWebsocketIPs, remoteIP);
	
	return Plugin_Continue;
}

public OnWebsocketReadyStateChanged(WebsocketHandle:websocket, WebsocketReadyState:readyState) {
	new iIndex = FindValueInArray(g_childrenWebsocketHandles, websocket);
	PrintToServer("[WS] allConnection readystate");
	if (iIndex == -1)
		return;
	
	if (readyState != State_Open)
		return;
	
	PrintToServer("[WS] Connection readystate");
	
	//Send game info
	decl String:packetBuffer[256], String:hostname[128], String:mapName[64];
	GetCurrentMap(mapName, sizeof(mapName));
	GetConVarString(g_hostname, hostname, sizeof(hostname));
	Format(packetBuffer, sizeof(packetBuffer), "I%s:%s:%s:%s", mapName, hostname, g_teamNameRed, g_teamNameBlu);
	
	Websocket_Send(websocket, SendType_Text, packetBuffer);
	
	//Send all players
	for (new i = 1; i <= MaxClients; i++) { // 0 = Worldspawn/"Console"
		if (IsClientInGame(i)) {
			
			Format(packetBuffer, sizeof(packetBuffer), "C%d:%d:%d:%d:%d:%d:%d:%d:%N", GetClientUserId(i), GetClientTeam(i), TF2_GetPlayerClass(i), 
				GetClientHealth(i), GetEntData(i, g_offset_m_iMaxHealth, 4), IsPlayerAlive(i), 0, 0, i);
			PrintToServer(packetBuffer);
			Websocket_Send(websocket, SendType_Text, packetBuffer);
		}
	}
	
	//Update spectator count
	new childrenCount = GetArraySize(g_childrenWebsocketHandles);
	Format(packetBuffer, sizeof(packetBuffer), "A%d", childrenCount);
	new WebsocketHandle:handle;
	for (new i = 0; i < childrenCount; i++) {
		handle = WebsocketHandle:GetArrayCell(g_childrenWebsocketHandles, i);
		if (Websocket_GetReadyState(handle) == State_Open)
			Websocket_Send(handle, SendType_Text, packetBuffer);
	}
	
	if(g_hUpdatePositions == INVALID_HANDLE)
	{
		g_hUpdatePositions = CreateTimer(POSITION_UPDATE_RATE, Timer_UpdatePlayerPositions, _, TIMER_REPEAT);
	}
	
	return;
}

public Action:Timer_UpdatePlayerPositions(Handle:timer, any:data)
{
	decl String:sBuffer[4096];
	
	// Update player positions
	Format(sBuffer, sizeof(sBuffer), "O");
	new Float:fOrigin[3], Float:fAngle[3];
	new Float:medicCharge;
	for(new i=1;i<=MaxClients;i++)
	{
		if(IsClientInGame(i) && IsPlayerAlive(i))
		{
			new userid = GetClientUserId(i);
			new weaponIndex = GetPlayerWeaponSlot(i, 1);
			if(strlen(sBuffer) > 1)
				Format(sBuffer, sizeof(sBuffer), "%s|", sBuffer);
			
			if (TF2_GetPlayerClass(i) == TFClass_Medic && weaponIndex > 0)
				medicCharge = GetEntPropFloat(weaponIndex, Prop_Send, "m_flChargeLevel")*100;
			else
				medicCharge = -1.0;
			GetClientAbsOrigin(i, fOrigin);
			GetClientEyeAngles(i, fAngle);
			Format(sBuffer, sizeof(sBuffer), "%s%d:%d:%d:%d:%d:%d", sBuffer, userid, RoundToNearest(fOrigin[0]), RoundToNearest(fOrigin[1]), RoundToNearest(fAngle[1]), GetClientHealth(i), RoundToNearest(medicCharge));
		}
	}
	
	if(strlen(sBuffer) == 1)
		return Plugin_Continue;
	
	SendToAllChildren(sBuffer);
	return Plugin_Continue;
}

public OnWebsocketMasterError(WebsocketHandle:websocket, const errorType, const errorNum)
{
	LogError("MASTER SOCKET ERROR: handle: %d type: %d, errno: %d", _:websocket, errorType, errorNum);
	g_serverSocket = INVALID_WEBSOCKET_HANDLE;
}

public OnWebsocketMasterClose(WebsocketHandle:websocket)
{
	g_serverSocket = INVALID_WEBSOCKET_HANDLE;
}

public OnChildWebsocketError(WebsocketHandle:websocket, const errorType, const errorNum)
{
	LogError("CHILD SOCKET ERROR: handle: %d, type: %d, errno: %d", _:websocket, errorType, errorNum);
	new iIndex = FindValueInArray(g_childrenWebsocketHandles, websocket);
	RemoveFromArray(g_childrenWebsocketHandles, iIndex);
	RemoveFromArray(g_childrenWebsocketIPs, iIndex);
	
	if(GetArraySize(g_childrenWebsocketHandles) == 0 && g_hUpdatePositions != INVALID_HANDLE)
	{
		KillTimer(g_hUpdatePositions);
		g_hUpdatePositions = INVALID_HANDLE;
	}
	
	// Inform others there's one spectator less!
	decl String:sBuffer[10];
	Format(sBuffer, sizeof(sBuffer), "A%d", GetArraySize(g_childrenWebsocketHandles));
	SendToAllChildren(sBuffer);
}

public OnWebsocketReceive(WebsocketHandle:websocket, WebsocketSendType:iType, const String:receiveData[], const dataSize)
{
	if(iType != SendType_Text)
		return;
	
	/*decl String:sBuffer[dataSize+4];
	Format(sBuffer, dataSize+4, "Z%s", receiveData);
	
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	new WebsocketHandle:hHandle;
	for(new i=0;i<iSize;i++)
	{
		hHandle = WebsocketHandle:GetArrayCell(g_childrenWebsocketHandles, i);
		if(hHandle != websocket && Websocket_GetReadyState(hHandle) == State_Open)
			Websocket_Send(hHandle, SendType_Text, sBuffer);
	}*/
	return;
}

public OnWebsocketDisconnect(WebsocketHandle:websocket)
{
	PrintToServer("disconnect");
	new iIndex = FindValueInArray(g_childrenWebsocketHandles, websocket);
	RemoveFromArray(g_childrenWebsocketHandles, iIndex);
	RemoveFromArray(g_childrenWebsocketIPs, iIndex);
	
	if(GetArraySize(g_childrenWebsocketHandles) == 0 && g_hUpdatePositions != INVALID_HANDLE)
	{
		KillTimer(g_hUpdatePositions);
		g_hUpdatePositions = INVALID_HANDLE;
	}
	
	// Inform others there's one spectator less!
	decl String:sBuffer[10];
	Format(sBuffer, sizeof(sBuffer), "A%d", GetArraySize(g_childrenWebsocketHandles));
	SendToAllChildren(sBuffer);
}

SendToAllChildren(const String:sData[])
{
	new iSize = GetArraySize(g_childrenWebsocketHandles);
	new WebsocketHandle:hHandle;
	for(new i=0;i<iSize;i++)
	{
		hHandle = WebsocketHandle:GetArrayCell(g_childrenWebsocketHandles, i);
		if(Websocket_GetReadyState(hHandle) == State_Open)
			Websocket_Send(hHandle, SendType_Text, sData);
	}
}
