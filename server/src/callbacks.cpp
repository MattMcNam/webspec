/*
 *  callbacks.h
 *  WebSpec project
 *  
 *  Copyright (c) 2013 Matthew McNamara
 *  BSD 2-Clause License
 *  http://opensource.org/licenses/BSD-2-Clause
 *
 */

#include "callbacks.h"

//=================================================================================
// Default HTTP callback
// Required by libwebsockets, but not required to actually do anything
//=================================================================================
int webspec_callback_http(struct libwebsocket_context *ctx, struct libwebsocket *wsi, 
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
	return 0;
}

//=================================================================================
// Callback for the 'webspec' protocol
// Manages spectator connections & sending Initial messages to new spectators
//=================================================================================
int webspec_callback(struct libwebsocket_context *ctx, struct libwebsocket *wsi, 
	enum libwebsocket_callback_reasons reason, void *user, void *in, size_t len)
{
	switch (reason) {
		case LWS_CALLBACK_ESTABLISHED:
		{
			Msg("[WS] New connection\n");
			// New connection
			ws_spectators.AddToTail(wsi);
			
			// Send basic game info to let client set up
			// MapName, Server name (may remove), current team names
			char *buffer = (char*)malloc(MAX_BUFFER_SIZE);
			char *mapName = (char*) STRING(gpGlobals->mapname);
			ConVarRef hostNameCVar = ConVarRef("hostname");
			string_t hostname;
			if (hostNameCVar.IsValid())
				hostname = MAKE_STRING(hostNameCVar.GetString());
			else
				hostname = MAKE_STRING("WebSpec Demo Server"); //Can't imagine when hostname would be invalid, but this is Source
			int length = snprintf(buffer, MAX_BUFFER_SIZE, "%c%s:%s:%s:%s", WSPacket_Init, mapName, STRING(hostname), STRING(ws_teamName[1]), STRING(ws_teamName[0]));

			SendPacketToOne(buffer, length, wsi);
			free(buffer);

			//Send connected players
			IPlayerInfo *playerInfo;
			for (int i=1; i<=gpGlobals->maxClients; i++) {
				playerInfo = playerInfoManager->GetPlayerInfo(engine->PEntityOfEntIndex(i));
				if (playerInfo != NULL && playerInfo->IsConnected()) {
					buffer = (char *)malloc(MAX_BUFFER_SIZE);
					int userid = playerInfo->GetUserID();
					int teamid = playerInfo->GetTeamIndex();
					int health = playerInfo->GetHealth();
					int maxHealth = playerInfo->GetMaxHealth();
					bool alive = !playerInfo->IsDead();
					string_t playerName = MAKE_STRING(playerInfo->GetName());
					
					//Pointer magic to get TF2 class
					CBaseEntity *playerEntity = serverGameEnts->EdictToBaseEntity(engine->PEntityOfEntIndex(i));
					int playerClass = *MakePtr(int*, playerEntity, WSOffsets::pCTFPlayer__m_iClass);

					float uberCharge = 0.0f;
					
					if (playerClass == TFClass_Medic) { 
						//Way more pointer magic to get ubercharge from medigun
						CBaseCombatCharacter *playerCombatCharacter = CBaseEntity_MyCombatCharacterPointer(playerEntity);
						CBaseCombatWeapon *slot1Weapon = CBaseCombatCharacter_Weapon_GetSlot(playerCombatCharacter, 1);
						
						uberCharge = *MakePtr(float*, slot1Weapon, WSOffsets::pCWeaponMedigun__m_flChargeLevel);
					}

					int length = snprintf(buffer, MAX_BUFFER_SIZE, "%c%d:%d:%d:%d:%d:%d:0:%d:%s", 'C', userid, teamid, playerClass,
						health, maxHealth, alive, Round(uberCharge*100.0f), STRING(playerName));

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
			ws_spectators.FindAndRemove(wsi);
		}
		default:
			break;
	}

	return 0;
}
