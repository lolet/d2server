/* ==========================================================
* d2warden
* https://github.com/lolet/d2warden
* ==========================================================
* Copyright 2011-2014 Bartosz Jankowski
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
* ========================================================== */

#include "stdafx.h"
#include "LPackets.h"
#include "LSpectator.h"
#include "Build.h"
#include <atomic>
#include "RC4.h"

/*
	Replacement for D2GAME.0xBB9F0
	(PacketData* pPacket<eax>)
*/
void  __stdcall OnDebugPacketReceive(PacketData* pPacket)
{
	std::atomic_uint nDebugClients;

	if (!*D2Vars.D2GAME_gpfnEventCallbacks)
		return;

	char data[16];
	char response[189];
	const char *errata = "Anyway. Go fuck yourself";

	switch (pPacket->aPacket[1])
	{
	case 0xFA:
		++nDebugClients;
		break;
	case 0xFB:
		--nDebugClients;
		break;
	case 0xFC:
		D2Funcs.D2NET_GetIpAddress(pPacket->ClientID, data, 16);

		sprintf_s(response, 189, "Connected from: %s", data);
		D2Funcs.D2NET_SendPacket(2, pPacket->ClientID, (BYTE*)response, strlen(response) + 1);
		break;
	case 0x1:
	case 0xFD:
		D2Funcs.D2NET_GetIpAddress(pPacket->ClientID, data, 16);
		Log("WARNING: (%s) wanted to send malicious packet!", data);

		sprintf_s(response, 189, "This is D2GS %s by Lolet (build %d). Compiled on %s, %s\n%s",
#ifdef VER_111B
			"1.11b",
#elif defined VER_113D
			"1.13d",
#endif
			__BUILDNO__, __DATE__, __TIME__, errata);

		D2Funcs.D2NET_SendPacket(2, pPacket->ClientID, (BYTE*)response, strlen(response) + 1);
		break;
	default:
		Log("WARNING: No action taken for received debug packet id (0x%x). Reason: No callback", pPacket->aPacket[1]);
		break;
	}


}

/*
	Replacement for D2GAME.0x673A0
	(BYTE *pPacket<ebx>, UnitAny *pUnit<esi>, Game *pGame, int nPacketLen)
	*/
int __stdcall OnPacketReceive(BYTE *pPacket, UnitAny *pUnit, Game *pGame, int nPacketLen)
{
	BYTE pType = pPacket[0];
	if (pType > 0x66)
		return MSG_HACK;
	PacketTbl* cbCallback = (PacketTbl*)&D2Vars.D2GAME_ClientPacketTable[pType];

	if (!cbCallback->Callback)
	{
		Log("WARNING: No action taken for received packet id (0x%x). Reason: No callback", pType);
		return MSG_HACK;
	}

	switch (pType)
	{
	case D2SRVMSG_LEFT_CLICK_ON_LOC:	//0x05
	case D2SRVMSG_RIGHT_CLICK_ON_LOC:	//0x0C
	case D2SRVMSG_HOLD_LEFT_CLICK_ON_LOC: //0x08
	case D2SRVMSG_HOLD_RIGHT_CLICK_ON_LOC: //0x0F
	{
		if (!pUnit || pUnit->dwMode == PLAYER_MODE_DEAD || pUnit->dwMode == PLAYER_MODE_DEATH || D2Funcs.D2COMMON_GetUnitState(pUnit, uninterruptable))
			return MSG_OK;

		if (pGame->nSyncTimer > 1)
			pGame->nSyncTimer = D2Funcs.FOG_GetTime();

		return OnClickLocation(pGame, pUnit, (SkillPacket*)pPacket, nPacketLen);
	}
	// left button
	case D2SRVMSG_LEFT_CLICK_ON_UNIT:			// 0x06
	case D2SRVMSG_SHIFT_LEFT_CLICK_ON_UNIT:		// 0x07
	case D2SRVMSG_HOLD_LEFT_CLICK_ON_UNIT:		// 0x09
	case D2SRVMSG_SHIFT_LEFT_CLICK_ON_UNIT_2:	// 0x0A
	// right button
	case D2SRVMSG_RIGHT_CLICK_ON_UNIT:			// 0x0D
	case D2SRVMSG_SHIFT_RIGHT_CLICK_ON_UNIT:	// 0x0E
	case D2SRVMSG_HOLD_RIGHT_CLICK_ON_UNIT:		// 0x10
	case D2SRVMSG_SHIFT_RIGHT_CLICK_ON_UNIT_2:	// 0x11
	{

		if (!pUnit || pUnit->dwMode == PLAYER_MODE_DEAD || pUnit->dwMode == PLAYER_MODE_DEATH || D2Funcs.D2COMMON_GetUnitState(pUnit, uninterruptable))
			return MSG_OK;

		if (pGame->nSyncTimer > 1)
			pGame->nSyncTimer = D2Funcs.FOG_GetTime();

		return OnClickUnit(pGame, pUnit, (SkillTargetPacket*)pPacket, nPacketLen);
	}
	case D2SRVMSG_RESSURECT: // Ressurect
	{
		if (!pUnit || pUnit->dwMode != PLAYER_MODE_DEAD)
			return MSG_OK;
		if (pGame->nSyncTimer > 1)
			pGame->nSyncTimer = D2Funcs.FOG_GetTime();
		return OnResurrect(pGame, pUnit, pPacket, nPacketLen);
	}
	case D2SRVMSG_OVERHEAD:
	case D2SRVMSG_CHAT:
	case D2SRVMSG_SELECT_SKILL: 
	case D2SRVMSG_UNKNOWN_0x43:
	{
		if (pGame->nSyncTimer > 1)
			pGame->nSyncTimer = D2Funcs.FOG_GetTime();

		return cbCallback->Callback(pGame, pUnit, pPacket, nPacketLen);
	}
	case D2SRVMSG_PARTY_OPTIONS_2: // Party relation
	{
		px5e * p = (px5e*)pPacket;
		if (nPacketLen != sizeof(px5e))
		{
			if (pUnit)
				Log("HACK: Malformed packet 0x5E. **VERY SUSPECT**. Received from *%s", pUnit->pPlayerData->pClientData->AccountName);
			return MSG_HACK;
		}
		if (p->nButton != PB_SPECATE)
		{
			if (!pUnit || pUnit->dwMode == PLAYER_MODE_DEAD || pUnit->dwMode == PLAYER_MODE_DEATH || D2Funcs.D2COMMON_GetUnitState(pUnit, uninterruptable))
				return MSG_OK;
		}
		else
		{
			if (!pUnit || pUnit->dwMode == PLAYER_MODE_DEAD || pUnit->dwMode == PLAYER_MODE_DEATH)
				return MSG_OK;
		}

		if (pGame->nSyncTimer > 1)
			pGame->nSyncTimer = D2Funcs.FOG_GetTime();

		return cbCallback->Callback(pGame, pUnit, pPacket, nPacketLen);
	}
	case D2SRVMSG_WARDEN_RESPONSE:
	{
		if (pGame->nSyncTimer > 1)
			pGame->nSyncTimer = D2Funcs.FOG_GetTime();

		return d2warden_0X66Handler(pGame, pUnit, pPacket, nPacketLen);
	}
	default:
	{
		if (!pUnit || pUnit->dwMode == PLAYER_MODE_DEAD || pUnit->dwMode == PLAYER_MODE_DEATH || D2Funcs.D2COMMON_GetUnitState(pUnit, uninterruptable))
			return MSG_OK;

		if (pGame->nSyncTimer > 1)
			pGame->nSyncTimer = D2Funcs.FOG_GetTime();

		return cbCallback->Callback(pGame, pUnit, pPacket, nPacketLen);
	}
	}

	return MSG_HACK;
}


//Valid for 0x06*, 0x07, 0x09*, 0x0A,  || 0x0D*, 0x0E, 0x10*, 0x11
DWORD __fastcall OnClickUnit(Game* pGame, UnitAny* pPlayer, SkillTargetPacket *ptPacket, DWORD PacketLen)
{
	int InRange = 0;
	if (PacketLen != 9)
		return 3;
	if (!pGame)
		return 3;
	if (pPlayer->dwType != UNIT_PLAYER)
		return 3;
	if (ptPacket->UnitType > 5)
		return 3;

	InRange = D2ASMFuncs::D2GAME_isUnitInRange(pGame, ptPacket->UnitId, ptPacket->UnitType, pPlayer, 50);
	if (InRange == 2)
	{
		DEBUGMSG("OnClickUnit: Over the range (2)")
			return MSG_ERROR;
	}
	if (InRange == 3)
	{
		DEBUGMSG("OnClickUnit: Over the range (3)");
		return MSG_HACK;
	}
	Skill * ptSkill = (ptPacket->Header == 6 || ptPacket->Header == 7 || ptPacket->Header == 9 || ptPacket->Header == 0xA) ? D2Funcs.D2COMMON_GetLeftSkill(pPlayer) : D2Funcs.D2COMMON_GetRightSkill(pPlayer);
	if (!ptSkill)
	{
		DEBUGMSG("%s: ptSkill not found!. Packet id is = %d", __FUNCTION__, ptPacket->Header);
		return 3;
	}
	int SkillId = D2Funcs.D2COMMON_GetSkillId(ptSkill, __FILE__, __LINE__);

	PlayerData* pPlayerData = pPlayer->pPlayerData;
	if (!pPlayerData) return 2;

	if (TeleChars[pPlayer->dwClassId] == FALSE && pPlayer->pGame->dwGameState == 0 && SkillId == 0x36)
	{
		SendMsgToClient(pPlayerData->pClientData, pPlayerData->pClientData->LocaleID == 10 ? "Teleport nie jest dozwolony dla tej klasy!" : "Teleport Is Not Allowed For This Character");
		return 0;
	}

	if (SkillId == 0x65 && !wcfgAllowHB)
	{
		SendMsgToClient(pPlayerData->pClientData, pPlayerData->pClientData->LocaleID == 10 ? "Swiety pocisk jest zabroniony na tym serwerze" : "Holy Bolt Is Not Allowed On This Server");
		return 0;
	}
	if (SkillId == 151 && !wcfgAllowNLWW)
	{
		SendMsgToClient(pPlayerData->pClientData, pPlayerData->pClientData->LocaleID == 10 ? "NLWW jest zabronione na tym serwerze" : "NLWW Is Not Allowed On This Server");
		return 0;
	}

	static int AttackCount;

	if (pGame->bFestivalMode == 1 && pPlayerData->CanAttack == 0 && (!isSafeSkill(SkillId) && SkillId != D2S_CHARGE))
	{
		if (AttackCount == 0)
		{
			SendMsgToClient(pPlayerData->pClientData, pPlayerData->pClientData->LocaleID == 10 ? "Wpisz #go aby rozpoczac runde!" : "Type #go to start round");
		}
		AttackCount++;
		if (AttackCount > 4) AttackCount = 0;
		return 0;
	}


	int nPierceIdx = D2Funcs.D2COMMON_GetBaseStatSigned(pPlayer, 328, 0);
	D2Funcs.D2COMMON_SetStat(pPlayer, 328, nPierceIdx + 1, 0);

	D2ASMFuncs::D2GAME_CastSkillOnUnit(pPlayer, ptSkill, pGame, ptPacket->UnitType, ptPacket->UnitId, (ptPacket->Header == 6 || ptPacket->Header == 9 || ptPacket->Header == 0xD || ptPacket->Header == 0x10) ? 1 : 0);

	UnitAny * pDest = D2ASMFuncs::D2GAME_FindUnit(pGame, ptPacket->UnitId, (BYTE)ptPacket->UnitType);
	if (pDest)
	{
		int mX = D2Funcs.D2COMMON_GetPathX(pDest->pPath);
		int mY = D2Funcs.D2COMMON_GetPathY(pDest->pPath);

		SPECTATOR_UpdatePositions(pGame, pPlayer, mX, mY);
	}
	else
	{
		DEBUGMSG("WARNING: pDest in %s was not found!", __FUNCTION__)
	}
	return 0;
}


DWORD __fastcall OnClickLocation(Game* pGame, UnitAny* pPlayer, SkillPacket *ptPacket, DWORD PacketLen)
{
	bool InRange = false;
	static int AttackCount;
	if (PacketLen != 5)
		return MSG_HACK; 
	if (!pGame)
		return MSG_HACK;
	if (pPlayer->dwType != UNIT_PLAYER)
		return MSG_HACK;

	WORD UnitX = D2Funcs.D2GAME_GetUnitX(pPlayer);
	WORD UnitY = D2Funcs.D2GAME_GetUnitY(pPlayer);

	int xOffset = UnitX - ptPacket->xPos;
	if (xOffset < 0) xOffset = -xOffset;
	int yOffset = UnitY - ptPacket->yPos;
	if (yOffset < 0) yOffset = -yOffset;

	if (yOffset < 50 && xOffset < 50) InRange = true;

	PlayerData* pPlayerData = pPlayer->pPlayerData;
	if (!pPlayerData)
	{
		DEBUGMSG("Didn't find a PlayerData, function %s, %d", __FUNCTION__, __LINE__);
		return MSG_ERROR;
	}

	if (InRange)
	{
		pPlayerData->GameFrame = pGame->GameFrame;
		Skill * ptSkill = (ptPacket->Header == 5 || ptPacket->Header == 8) ? D2Funcs.D2COMMON_GetLeftSkill(pPlayer) : D2Funcs.D2COMMON_GetRightSkill(pPlayer);
		if (!ptSkill) return MSG_HACK;
		int SkillId = D2Funcs.D2COMMON_GetSkillId(ptSkill, __FILE__, __LINE__);

		if (TeleChars[pPlayer->dwClassId] == FALSE && pPlayer->pGame->dwGameState == 0 && SkillId == D2S_TELEPORT)
		{
			SendMsgToClient(pPlayerData->pClientData, pPlayerData->pClientData->LocaleID == 10 ? "Teleport nie jest dozwolony dla tej klasy!" : "Teleport Is Not Allowed For This Character");
			return MSG_OK;
		}

		if (SkillId == D2S_HOLYBOLT && !wcfgAllowHB) {
			SendMsgToClient(pPlayerData->pClientData, pPlayerData->pClientData->LocaleID == 10 ? "Swiety pocisk jest zabroniony na tym serwerze" : "Holy Bolt Is Not Allowed On This Server");
			return MSG_OK;
		}

		int nPierceIdx = D2Funcs.D2COMMON_GetBaseStatSigned(pPlayer, 328, 0);
		D2Funcs.D2COMMON_SetStat(pPlayer, 328, nPierceIdx + 1, 0);

		if (pGame->bFestivalMode && !pPlayerData->CanAttack && !isSafeSkill(SkillId))
		{
			if (AttackCount == 0)
			{
				SendMsgToClient(pPlayerData->pClientData, pPlayerData->pClientData->LocaleID == 10 ? "Wpisz #go aby rozpoczac runde!" : "Type #go to start round");
			}
			AttackCount++;
			if (AttackCount > 4)
				AttackCount = 0;
			return MSG_OK;
		}

		D2ASMFuncs::D2GAME_CastSkill(pPlayer, ptSkill, pGame, ptPacket->xPos, ptPacket->yPos);
		if (SkillId == D2S_TELEPORT || SkillId == D2S_CHARGE || SkillId == D2S_WHIRLWIND)
			SPECTATOR_UpdatePositions(pGame, pPlayer, ptPacket->xPos, ptPacket->yPos);
		else
			SPECTATOR_UpdatePositions(pGame, pPlayer, UnitX, UnitY);

		if (!wcfgDetectTrick)
			return MSG_OK;

		WardenClient_i ptWardenClient = GetClientByID(pPlayerData->pClientData->ClientID);
		if (ptWardenClient == hWarden.Clients.end()) return 0;

		if (GetTickCount() > ptWardenClient->UIModesTime + 500) { UNLOCK return 0; }

		if ((ptWardenClient->UIModes[UI_CHARACTER] || ptWardenClient->UIModes[UI_QUEST]) && (ptWardenClient->MouseXPosition >= 0 && ptWardenClient->MouseXPosition <= 200) && (ptWardenClient->MouseYPosition >= 0 && ptWardenClient->MouseYPosition <= 550))
		{
			if (ptWardenClient->DebugTrick)
				SendMsgToClient(ptWardenClient->ptClientData, "Trick (Left window) X=%d Y=%d, LAG= %d ms", ptWardenClient->MouseXPosition, ptWardenClient->MouseYPosition, GetTickCount() - ptWardenClient->UIModesTime);
			Log("HACK: %s (*%s) used Polish GA Trick [%s]!, skill : %s XY=[%d,%d]", ptWardenClient->CharName.c_str(), ptWardenClient->AccountName.c_str(), ptWardenClient->UIModes[UI_CHARACTER] ? "Character Stats" : "Quests", ConvertSkill(SkillId).c_str(), ptWardenClient->MouseXPosition, ptWardenClient->MouseYPosition);
		}
		else
			if ((ptWardenClient->UIModes[UI_INVENTORY] || ptWardenClient->UIModes[UI_SKILL]) && (ptWardenClient->MouseXPosition >= 600 && ptWardenClient->MouseXPosition <= 800) && (ptWardenClient->MouseYPosition >= 0 && ptWardenClient->MouseYPosition <= 550))
			{
				if (ptWardenClient->DebugTrick)
					SendMsgToClient(ptWardenClient->ptClientData, "Trick (Right window) X=%d Y=%d, LAG = %d ms", ptWardenClient->MouseXPosition, ptWardenClient->MouseYPosition, GetTickCount() - ptWardenClient->UIModesTime);
				Log("HACK: %s (*%s) used Polish GA Trick [%s]!, skill : %s XY=[%d,%d]", ptWardenClient->CharName.c_str(), ptWardenClient->AccountName.c_str(), ptWardenClient->UIModes[UI_INVENTORY] ? "Inventory" : "Skill Tree", ConvertSkill(SkillId).c_str(), ptWardenClient->MouseXPosition, ptWardenClient->MouseYPosition);
			}
		UNLOCK
			return MSG_OK;
	}
	else
	{
		if ((signed int)(pGame->GameFrame - pPlayerData->GameFrame) > 25)
		{
			ClientData * pClient = pPlayerData->pClientData;
			if (pClient)
			{
				ReassignPacket hReassign = { 0 };
				hReassign.Header = 0x15;
				hReassign.UnitId = pPlayer->dwUnitId;
				hReassign.xPos = UnitX;
				hReassign.yPos = UnitY;
				hReassign.Reassign = 1;

				D2ASMFuncs::D2GAME_SendPacket(pClient, (BYTE*)&hReassign, 11);
			}
		}
		return MSG_UNK;
	}
	return MSG_HACK;
}

DWORD __fastcall OnRunToLocation(Game* pGame, UnitAny* pPlayer, SkillPacket *ptPacket, DWORD PacketLen)
{
	bool InRange = false;
	if (PacketLen != 5) return MSG_HACK; // zwroc hack
	if (!pGame) return MSG_HACK;
	if (!pPlayer || pPlayer->dwType != UNIT_PLAYER) return MSG_HACK;

	WORD UnitX = D2Funcs.D2GAME_GetUnitX(pPlayer);
	WORD UnitY = D2Funcs.D2GAME_GetUnitY(pPlayer);

	int xOffset = UnitX - ptPacket->xPos;
	if (xOffset < 0) xOffset = -xOffset;
	int yOffset = UnitY - ptPacket->yPos;
	if (yOffset < 0) yOffset = -yOffset;

	if (yOffset < 50 && xOffset < 50) InRange = true;

	PlayerData* pPlayerData = pPlayer->pPlayerData;
	if (!pPlayerData)
	{
		Log("Didn't find a PlayerData, function %s, %d", __FUNCTION__, __LINE__);
		return MSG_ERROR;
	}
	if (pPlayerData->pTrade)
	{
		WardenClient_i ptWardenClient = GetClientByID(pPlayerData->pClientData->ClientID);
		if (ptWardenClient == hWarden.Clients.end()) return MSG_OK;
		if (!ptWardenClient->DupeDetected)
		{
			Log("HACK: %s (*%s) tryed to move while in trade [Probably *nice* dupe try]", ptWardenClient->CharName.c_str(), ptWardenClient->AccountName.c_str());
			ptWardenClient->DupeDetected = 1;
		}
		UNLOCK
			return MSG_HACK;
	}
	if (InRange)
	{
		SPECTATOR_UpdatePositions(pGame, pPlayer, UnitX, UnitY);
		pPlayerData->GameFrame = pGame->GameFrame;
		D2Funcs.D2GAME_SetUnitModeXY(pGame, pPlayer, 0, PLAYER_MODE_RUN, ptPacket->xPos, ptPacket->yPos, 0);
		return MSG_OK;
	}
	else
	{
		if ((signed int)(pGame->GameFrame - pPlayerData->GameFrame) > 25)
		{
			ClientData * pClient = pPlayerData->pClientData;
			if (pClient)
			{
				ReassignPacket hReassign = { 0 };
				hReassign.Header = 0x15;
				hReassign.UnitId = pPlayer->dwUnitId;
				hReassign.xPos = UnitX;
				hReassign.yPos = UnitY;
				hReassign.Reassign = 1;

				D2ASMFuncs::D2GAME_SendPacket(pClient, (BYTE*)&hReassign, 11);
			}
		}
		return MSG_UNK;
	}
	return MSG_HACK;
}

DWORD __fastcall d2warden_0X66Handler(Game* ptGame, UnitAny* ptPlayer, BYTE *ptPacket, DWORD PacketLen) // packet 0x66 -> response for Warden question
{
	if (!ptPlayer)
	{
		DEBUGMSG("WardenPacket: ptPlayer == null!");
		return MSG_HACK;
	}

	DWORD ClientID = ptPlayer->pPlayerData->pClientData->ClientID;

	if (PacketLen < 3)
	{
		DEBUGMSG("WardenPacket: PacketLen < 3 !");
		return MSG_HACK;
	}

	if (!ClientID)
	{
		DEBUGMSG("WardenPacket: No client id!");
		return MSG_HACK;
	}


	WardenClient_i i = GetClientByID(ClientID);
	if (i != hWarden.Clients.end())
	{
		i->pWardenPacket.ReceiveTime = GetTickCount();
		i->pWardenPacket.PacketLen = ptPacket[2] * 256 + ptPacket[1];

		if (i->pWardenPacket.PacketLen == 0 || i->pWardenPacket.PacketLen > 512) // Taka jest maksymalna wielkosc pakietu obslugiowanego przez d2
		{
			DEBUGMSG("WardenPacket: Packet size exceeds 512 bytes!");
			UNLOCK
				return MSG_HACK;
		}

		BYTE *ThePacket = new BYTE[i->pWardenPacket.PacketLen];
		if (!ThePacket)
		{
			Log("WardenPacket: No memory to allocate packet data!");
			UNLOCK
				return MSG_HACK;
		}

		memcpy(ThePacket, ptPacket + 3, i->pWardenPacket.PacketLen);
		i->pWardenPacket.ThePacket = ThePacket;

		rc4_crypt(i->RC4_KEY_0X66, i->pWardenPacket.ThePacket, i->pWardenPacket.PacketLen);
		//DEBUGMSG("WardenPacket: Received answer in %d ms", i->pWardenPacket.SendTime ? (i->pWardenPacket.ReceiveTime - i->pWardenPacket.SendTime) : 0);
		i->NextCheckTime = GetTickCount();
		UNLOCK
			//DEBUGMSG("WardenPacket: Triggering the check event...");
			SetEvent(hWardenCheckEvent);
		return MSG_OK; // Wszystko OK!
	}
	else
	{
		DEBUGMSG("WardenPacket: Client %d, %s (*%s) is not in WardenQueue!!", ClientID, ptPlayer->pPlayerData->pClientData->CharName, ptPlayer->pPlayerData->pClientData->AccountName);

#ifdef _ENGLISH_LOGS
		Log("WardenPacket: Unexpected packet from player %s (*%s)! Returning an error..", ptPlayer->pPlayerData->szName, ptPlayer->pPlayerData->pClientData->AccountName);
#else
		Log("WardenPacket: Nieoczekiwany pakiet od gracza %s (*%s)! Zwracam blad...", ptPlayer->pPlayerData->szName, ptPlayer->pPlayerData->pClientData->AccountName);
#endif
		return MSG_HACK;
	}

}

// Replacement for 0x41 parse (1.11b: D2Game.6FC51F60)
DWORD __fastcall OnResurrect(Game *pGame, UnitAny *pPlayer, BYTE *aPacket, DWORD PacketSize)
{
	ClientData *pClientData = 0;
	DEBUGMSG(__FUNCTION__)
	if (PacketSize != 1)
		return MSG_HACK;

	if (!pPlayer || pPlayer->dwMode != PLAYER_MODE_DEAD)
		return MSG_OK;


	if (pPlayer->dwType == UNIT_PLAYER)
		pClientData = pPlayer->pPlayerData->pClientData;

	if (!pClientData)
		return MSG_HACK;
	if (pClientData->PlayerStatus & 4)
	{
		D2ASMFuncs::D2GAME_BroadcastLeavingEvent(pClientData, pGame, EVENT_LEFT);
		return MSG_OK;
	}

	if ((GetTickCount() - pPlayer->pPlayerData->tDeathTime) <= (wcfgRespawnTimer * 1000))
		return MSG_OK;

	if (pPlayer->pSkills)
	{
		for (Skill* s = pPlayer->pSkills->pFirstSkill; s; s = s->pNextSkill)
		{
			WORD nSkill = D2Funcs.D2COMMON_GetSkillId(s, __FILE__, __LINE__);
			int nState = D2Funcs.D2COMMON_GetStateNoBySkillId(nSkill);
			if (nState > 0)
			{
				D2Funcs.D2COMMON_SetGfxState(pPlayer, nState, 1);
				D2Funcs.D2COMMON_RefreshAura(pPlayer, nSkill);
			}
		}
	}

	int dwMaxLife = D2Funcs.D2COMMON_GetUnitMaxLife(pPlayer);
	int dwMaxMana = D2Funcs.D2COMMON_GetUnitMaxMana(pPlayer);
	int dwMaxStamina = D2Funcs.D2COMMON_GetStatSigned(pPlayer, STAT_MAXSTAMINA, 0);

	D2Funcs.D2COMMON_SetStat(pPlayer, STAT_HP, dwMaxLife, 0);
	D2Funcs.D2COMMON_SetStat(pPlayer, STAT_MANA, dwMaxMana, 0);
	D2Funcs.D2COMMON_SetStat(pPlayer, STAT_STAMINA, dwMaxStamina, 0);

	D2Funcs.D2GAME_UpdatePlayerStats(pPlayer, STAT_HP, dwMaxLife, pPlayer);
	D2Funcs.D2GAME_UpdatePlayerStats(pPlayer, STAT_MANA, dwMaxMana, pPlayer);
	D2Funcs.D2GAME_UpdatePlayerStats(pPlayer, STAT_STAMINA, dwMaxStamina, pPlayer);
	pPlayer->dwFlags |= UNITFLAG_SELECTABLE;				  // Make unit selectable

	D2Funcs.D2COMMON_SetGfxState(pPlayer, uninterruptable, 1);   // Uninterrupable
	D2Funcs.D2COMMON_SetGfxState(pPlayer, uninterruptable, 0);

	int aLevel = D2Funcs.D2COMMON_GetTownLevel(pPlayer->dwAct);

	//Room1* aRoom = D2Funcs.D2COMMON_GetRoomXYByLevel(pRoom->pAct, aLevel, 0, &aX, &aY, 2);
	//D2ASMFuncs::D2GAME_TeleportUnit(aX, aY, aRoom, pGame, pUnit);

	D2ASMFuncs::D2GAME_MoveUnitToLevelId(pPlayer, aLevel, pGame);
	D2Funcs.D2GAME_SetUnitModeXY(pGame, pPlayer, 0, PLAYER_MODE_STAND_OUTTOWN, 0, 0, 1);

	Skill* pLeftSkill = D2Funcs.D2COMMON_GetLeftSkill(pPlayer);
	Skill* pRightSkill = D2Funcs.D2COMMON_GetRightSkill(pPlayer);
	if (pLeftSkill)
	{
		DWORD nFlags = pLeftSkill->dwFlags;
		WORD nSkill = D2Funcs.D2COMMON_GetSkillId(pLeftSkill, __FILE__, __LINE__);

		D2Funcs.D2GAME_SetMonSkill(pPlayer, 1, nSkill, nFlags);
	}

	if (pRightSkill)
	{
		DWORD nFlags = pRightSkill->dwFlags;
		WORD nSkill = D2Funcs.D2COMMON_GetSkillId(pRightSkill, __FILE__, __LINE__);
		D2Funcs.D2GAME_SetMonSkill(pPlayer, 0, nSkill, nFlags);
	}

	return MSG_OK;
}