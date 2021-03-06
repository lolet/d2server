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

#ifndef __LPACKETS_H__
#define __LPACKETS_H__

void  __stdcall OnCreatePacketReceive(PacketData* pPacket);
int __stdcall OnPacketReceive(BYTE *pPacket, UnitAny *pUnit, Game *pGame, int nPacketLen);
void  __stdcall OnDebugPacketReceive(PacketData* pPacket);


DWORD __fastcall OnRunToLocation(Game* ptGame, UnitAny* ptPlayer, SkillPacket *ptPacket, DWORD PacketLen);
DWORD __fastcall OnClickLocation(Game* ptGame, UnitAny* ptPlayer, SkillPacket *ptPacket, DWORD PacketLen);
DWORD __fastcall OnClickUnit(Game* ptGame, UnitAny* ptPlayer, SkillTargetPacket *ptPacket, DWORD PacketLen);

DWORD __fastcall OnResurrect(Game *pGame, UnitAny *pPlayer, BYTE *aPacket, DWORD PacketSize);

#endif