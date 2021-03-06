/* ==========================================================
 * d2warden
 * https://github.com/lolet/d2warden
 * ==========================================================
 * Copyright 2011-2013 Bartosz Jankowski
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

#ifndef __D2STUBS__H_
#define __D2STUBS__H_

namespace D2Stubs 
{
	void D2GAME_TestCrash_STUB();
	void UBERQUEST_SpawnMonsters_STUB();
	BYTE __fastcall GetActByLevelNo_STUB1(DWORD nLevel);
	BYTE __fastcall GetActByLevelNo_STUB2(DWORD nLevel);
	
	void D2GAME_OnCustomFunc_STUB();
	int __stdcall D2COMMON_GetMercCost(UnitAny* pPlayer);
	void D2COMMON_GetItemCost_STUB();
	void __fastcall OnDebugPacketReceive_STUB();
	void __fastcall D2GAME_IsUnitDead_STUB();
	void __stdcall D2GAME_OnPacketReceive_STUB();
	void __fastcall D2GAME_OnCreateDamage_STUB();
	void __fastcall D2GAME_LastHitIntercept_STUB();
	void __fastcall D2GAME_OnJoinGame_STUB();

	void __fastcall D2GAME_OnEventSend_STUB();
	void __fastcall D2GAME_LogHack_STUB();
	void __fastcall D2GAME_SendKillsNULL_STUB();
	void __fastcall D2GAME_OnUseItem_STUB();
	void __fastcall D2GAME_OnMonsterDeath_STUB();
	void __fastcall D2GAME_PartyExp_STUB();
	void __fastcall D2GAME_FreeNodes_STUB(Game *ptGame);
	void __stdcall D2GAME_SetNodeParented_STUB(UnitNode* ptNode);
	void __stdcall D2GAME_SetUnitsNode_STUB(UnitAny* ptUnit, UnitNode* ptNode);
	void __fastcall D2GAME_FreeNode_STUB(Game *ptGame, UnitAny *ptUnit);
	void __fastcall D2GAME_FreeChildNode_STUB(Game *ptGame, UnitAny *ptUnit);
	void __stdcall D2GAME_AssignNode_STUB(int dwZero);
	BOOL __fastcall D2GAME_PermStore_STUB();
	void __fastcall D2GAME_GameEnter_STUB();
	void __fastcall D2GAME_GameDestroy_STUB();
	void __fastcall D2GAME_DeathMsg_STUB(UnitAny* ptKiller, void * BitMask, Game * ptGame);
	int __stdcall D2GAME_Ressurect_STUB();
	void __fastcall D2GAME_NPCHeal_STUB();
	void __fastcall D2GAME_Chat_STUB();
	int __fastcall D2GAME_SendStatToOther_STUB();
	void __fastcall D2NET_ReceivePacket_STUB();
	DWORD __fastcall D2GAME_SetDRCap_STUB();
	void __fastcall D2GAME_OnManaLeech_STUB();
	void __fastcall D2GAME_OnLifeLeech_STUB();
	void __fastcall D2GAME_OnPlayerModeChange_I();
	UnitAny* __fastcall D2GAME_FindUnit_STUB();
	void __fastcall D2GAME_AddStat_STUB(int nStat, UnitAny *pUnit, int nValue);
	BOOL __fastcall D2GAME_GoldTransaction_STUB(UnitAny *pPlayer, int nCost);
}

// -- ASM SUBS -- 
namespace D2ASMFuncs 
{
void __stdcall D2GAME_UpdateRoomUnits(Game* pGame);
void __fastcall D2GAME_RemovePets(Game* pGame, UnitAny* pPlayer);
void __stdcall D2GAME_BroadcastLeavingEvent(ClientData *ptClient, Game *ptGame, BYTE MsgType);
void __fastcall D2GAME_SetPlayerUnitModeTarget(Game *pGame, UnitAny *pUnit, Skill *pSkill, int nMode, int UnitType, int UnitId, int bAllowReEnter);
void __fastcall D2GAME_UpdateClientInventory(ClientData* pClient, UnitAny* pPlayer);
void __fastcall D2GAME_DeleteTimer(Game* pGame, Timer* pTimer);
void __fastcall D2GAME_DeleteTimers(Game* pGame, UnitAny* pUnit);
char __fastcall D2GAME_MoveItems(Game* pGame, UnitAny* pSource, UnitAny* pDest);
int __fastcall D2GAME_RestoreItems(Game *pGame, BYTE *pSaveBuffer,UnitAny *pPlayer, int nItemSize, int nBufferMax, BOOL bUnk, int *BufferLen);
void __fastcall D2GAME_AddGold(UnitAny* pPlayer, Game *pGame, int nGold);
void __fastcall D2GAME_UpdateQuantity(UnitAny* pPlayer);
void __fastcall D2GAME_UpdateItemQuantity(UnitAny* pPlayer, UnitAny* pItem);
void __fastcall D2GAME_RemoveBonuses(Game* pGame, UnitAny* pPlayer);
int __fastcall D2GAME_SaveItems(BYTE *pBuffer, BOOL _2, Game *ptGame, UnitAny *ptUnit, Inventory *pInv, int nMaxSize, BOOL bInTrade, BOOL bQuit, BOOL bUnk);
void __fastcall D2GAME_StopSequence(UnitAny* pUnit);
void __fastcall D2GAME_RemoveBuffs(Game* pGame, UnitAny* pUnit);
void __fastcall D2GAME_ResetTimers(Game* pGame, UnitAny* pUnit);
void __fastcall D2GAME_RemoveInteraction(Game* pGame, UnitAny* pUnit);
void __fastcall D2GAME_CastSkill(UnitAny *ptUnit, Skill *ptSkill, Game *pGame, int xPos, int yPos);
int __fastcall D2GAME_CastSkillOnUnit(UnitAny *ptUnit, Skill *ptSkill, Game *pGame, DWORD UnitType, DWORD UnitId, BOOL bRepeat);
UnitAny* __fastcall D2GAME_SpawnMonsterInCurrentRoom(int MIdx, int a2, Game *ptGame, Room1 *pRoom, int xPos, int yPos, int a9, int a1);
StatList* __fastcall D2GAME_CreateAuraStatList(CurseStateParam *pCurse);
void __fastcall D2GAME_RemoveItem(Game* pGame, UnitAny* pUnit, UnitAny* pItem);
void __stdcall D2GAME_UpdateBonuses(UnitAny *pUnit);
void __fastcall D2GAME_SetSkills(int SkillId, UnitAny* pUnit, int SkillLvl, int a4);
void __stdcall D2GAME_Send0x21_UpdateSkills(ClientData* pClient, int SkillId, UnitAny* pUnit, BYTE a3, BYTE a4);
void __fastcall D2GAME_LevelAwards(UnitAny *pUnit, Game* pGame);
int __stdcall D2GAME_GetExpGained(int PlayerExp, UnitAny *pPlayer, int PlayerLvl, Game *pGame, int MonLvl);
void __fastcall D2GAME_ForEachInParty(UnitAny *pMonster, Game *pGame, void (__fastcall *pCallBack)(Game *pGame, UnitAny *pPartyMember, void *pData), void *pData);
void __fastcall D2GAME_ForEach(Game *pGame, void (__fastcall *pCallBack)(Game *, UnitAny*, void *), void *pData);
int __fastcall D2GAME_NodesUnk(UnitAny* pUnit1, int _1, UnitAny* pUnit2);
int __fastcall D2GAME_GetDistanceFromXY(UnitAny* pUnit, int aX, int aY);
UnitAny* __fastcall D2GAME_DupeItem(Game* ptGame, UnitAny* ptItem, BOOL bInitUnit);
int __fastcall D2GAME_DeleteItem(Game* ptGame, UnitAny* ptItem);
void __fastcall D2GAME_UpdateClient(WORD ItemFormat, UnitAny *ptItem, ClientData *pClient, BYTE SomeBool, int StatNo, int StatValue);
void __fastcall D2GAME_DropItem(UnitAny* ptItem, Room1* ptRoom, Game* ptGame, UnitAny* ptOwner, int xPos, int yPos);
UnitAny* __stdcall D2GAME_CreateItem(int iLvl, int a2, int a3, UnitAny* pOwner, int ItemIdx, Game* ptGame, int InitMode, int iQuality, BOOL Personalize, int LoSeed, int HiSeed);
Room1* __fastcall D2GAME_FindFreeCoords(D2POINT* Desired, Room1* ptRoom, D2POINT* Output, BOOL Unk);
Room1* __fastcall D2GAME_CheckXYOccupy(Room1* ptRoom, int xPos, int yPos);
int __fastcall D2GAME_isUnitInRange(Game *pGame, DWORD UnitId, DWORD UnitType, UnitAny *pUnit, int Range);
void __fastcall D2GAME_CreatePlayer(Game* ptGame, ClientData* ptClient, int Unk, UnitAny** out);
short __fastcall D2GAME_GetPartyID(UnitAny* ptUnit);
int __stdcall D2COMMON_GetUnitX(Path* ptPath);
int __stdcall D2COMMON_GetUnitY(Path* ptPath);
void __fastcall D2GAME_MoveUnitToLevelId(UnitAny *ptUnit, int LevelId, Game *ptGame);
int __fastcall D2GAME_TeleportUnit(int pX, int pY, Room1 *pRoom1, Game *pGame, UnitAny *pUnit);
Game* __fastcall D2GAME_GetGameByClientID(DWORD ClientID);
void __fastcall D2GAME_LeaveCriticalSection(Game* pGame);
DWORD __fastcall D2GAME_Send0XAEPacket(void *ptPlayer,DWORD Length,DWORD *Packet);
UnitAny* __stdcall D2GAME_FindUnit(Game* ptGame, DWORD dwUnitId, BYTE dwUnitType);
DWORD __fastcall D2GAME_SendPacket(ClientData *pClientData, BYTE *aPacket,int aLen);
}

#endif