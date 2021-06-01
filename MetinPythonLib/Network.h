#pragma once
#include <Windows.h>
#include <iostream>
#include "utils.h"
#include <set>
#include "DetoursHook.h"
#include <algorithm>

typedef bool(__thiscall *tSendPacket)(DWORD classPointer, int size, void* buffer);
typedef bool(__thiscall* tSendSequencePacket)(DWORD classPointer);
typedef bool(__thiscall* tBackground_CheckAdvancing)(DWORD classPointer, void* instanceBase);
typedef bool(__thiscall* tInstanceBase_CheckAdvancing)(DWORD classPointer);


enum {
	CHAR_STATE_FUNC_STOP = 0,
	CHAR_STATE_FUNC_WALK = 1,
	CHAR_STATE_FUNC_ATTACK = 3,

	CHAR_STATE_ARG_HORSE_ATTACK1 = 14,
	CHAR_STATE_ARG_HORSE_ATTACK2 = 15,
	CHAR_STATE_ARG_HORSE_ATTACK3 = 16,

	CHAR_STATE_ARG_COMBO_ATTACK1 = 14,
	CHAR_STATE_ARG_COMBO_ATTACK2 = 15,
	CHAR_STATE_ARG_COMBO_ATTACK3 = 16,
	CHAR_STATE_ARG_COMBO_ATTACK4 = 17,

	COMBO_SKILL_ARCH = 0
};

enum {
	SUCESS_ON_FISHING = 3,
	UNSUCESS_ON_FISHING = 0
};

enum {
	PHASE_LOADING = 4,
	PHASE_CHARACTER_CHOSE = 2,
	PHASE_GAME = 5
};




namespace PacketHeaders {
	enum {
		//FROM SERVER
		HEADER_GC_CHARACTER_ADD = 125,
		//HEADER_GC_MAIN_CHARACTER_MOVE = 11,
		HEADER_GC_CHARACTER_DEL = 84,
		//HEADER_GC_ITEM_USE = 86, Mostelikely chat
		HEADER_GC_DEAD = 124,
		HEADER_GC_PHASE = 255,
		HEADER_GC_CHARACTER_MOVE = 85,
		HEADER_GC_MAIN_CHARACTER = 75,
		HEADER_GC_CHAT = 121,
		HEADER_GC_FISHING = 42,
		HEADER_GC_ITEM_GROUND_ADD = 117,
		HEADER_GC_ITEM_GROUND_DEL = 129,
		HEADER_GC_SHOP_SIGN = 26,
		HEADER_CG_DIG_MOTION = 7,
		//91 SEND CHAT INFO
		//130 Something related to invetory counter


		//TO SERVER
		HEADER_CG_ITEM_PICKUP = 20,
		HEADER_CG_SEND_LOGIN = 68,
		HEADER_CG_SEND_SEQUENCE = 0,
		HEADER_CG_SEND_CHAT = 3,
		HEADER_CG_CHARACTER_MOVE = 11,//19, 
		HEADER_CG_FISHING = 14,
		HEADER_CG_SHOOT = 4,
		HEADER_CG_SELECT_TARGET = 82,
		HEADER_CG_BATTLE_ATTACK = 85,
		HEADER_CG_ADD_FLY_TARGETING = 77
		//Packet 77 something related before attacking
	};
}


struct Packet {
	Packet(int size, void* buffer);
	BYTE header;
	int data_size;
	BYTE* data;
};



#pragma pack(push, 1)
template<class T>
void fillPacket(Packet*p, T* _struct) {
	ZeroMemory(_struct, sizeof(T));
	int size = std::min<int>(p->data_size, sizeof(T));
	memcpy(_struct, p->data, size);
}

struct DeadPacket
{
	DWORD	vid;
};

struct GroundItemAddPacket {
	long x;
	long y;
	long z;

	DWORD VID;
	DWORD itemIndex;
	DWORD playerVID;
};

struct GroundItemDeletePacket {
	DWORD vid;
};

struct PickupPacket {
	BYTE header = PacketHeaders::HEADER_CG_ITEM_PICKUP;
	DWORD vid;
};

struct PacketDigMotion
{
	DWORD vid;
	DWORD target_vid;
	BYTE count;
};

struct CharacterStatePacket
{
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
};

struct RegisterShopPacket
{
	DWORD		dwVID;
};

struct AddFlyTargetingPacket
{
	BYTE		header = PacketHeaders::HEADER_CG_ADD_FLY_TARGETING;
	DWORD		dwTargetVID;
	long		lX;
	long		lY;
};

struct ShootPacket
{
	BYTE		header = PacketHeaders::HEADER_CG_SHOOT;
	BYTE		type;
};

struct MainCharacterPacket
{
	DWORD       dwVID;
	WORD		wRaceNum;
	char        szName[25];
	long        lX, lY, lZ;
	BYTE		bySkillGroup;
};

struct StartFishing
{
	BYTE		header = PacketHeaders::HEADER_CG_FISHING;
	WORD		u1 = 6;   //6
	BYTE		u2 = 6;	  //6
	WORD		direction;
};

struct StopFishing
{
	BYTE		header = PacketHeaders::HEADER_CG_FISHING;
	WORD        u1 = 12;	  //12
	BYTE		u2 = 8;   //8
	DWORD		type;	  //0 - unsuccess, 3 sucess
	float		timeLeft; //seconds
};


struct ChangePhasePacket
{
	BYTE phase;
};

struct CharacterMovePacket
{
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		bRot;
	DWORD		dwVID;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
	DWORD		dwDuration;
};

struct DeletePlayerPacket
{
	DWORD	dwVID;
};


struct PlayerCreatePacket {

	//DEFAULT
	DWORD	dwVID;
	float	angle;
	long	x;
	long	y;
	long	z;
	DWORD	dwLevel;
	WORD	uknown;
	BYTE	bType;
	WORD	wRaceNum;
	BYTE	bMovingSpeed;
	BYTE	bAttackSpeed;
	BYTE	bStateFlag;
	DWORD	dwAffectFlag[2];
	/*DWORD dwVID;
	DWORD uknown1;
	DWORD uknown2;
	DWORD uknown3;
	CHAR uknown4;
	DWORD uknown5;
	DWORD uknown6;
	BYTE uknown7;
	BYTE uknown8;
	CHAR uknown9;
	CHAR uknown10;
	DWORD uknown11;
	DWORD uknown12;*/

};
#pragma pack(pop)

//Packet filter
#define INBOUND 1
#define OUTBOUND 0

#define ONLY_INCLUDED_FILTER 1
#define ONLY_EXCLUDED_FILTER 0
typedef bool PACKET_TYPE;

//Packet filter
void openConsole();
void closeConsole();
void startFilterPacket();
void stopFilterPacket();
void removeHeaderFilter(BYTE header, PACKET_TYPE t);
void addHeaderFilter(BYTE header, PACKET_TYPE t);
void clearPacketFilter(PACKET_TYPE t);
void setFilterMode(PACKET_TYPE t,bool val);
std::set<BYTE>* getPacketFilter(PACKET_TYPE t);


//Hook Functions
bool __stdcall __RecvPacket(DWORD returnFunction, bool return_value, int size, void* buffer);
bool __stdcall __SendPacket(DWORD classPointer,DetoursHook<tSendPacket>* hook, void* retAddress,int size, void*buffer);
bool __fastcall __SendSequencePacket(DWORD classPointer);
bool __fastcall __BackgroundCheckAdvanced(DWORD classPointer, DWORD EDX, void* instanceBase); //waithack buildings
bool __fastcall __InstanceBaseCheckAdvanced(DWORD classPointer);//waithack envoirnment_monsters 


//SendPacket Functions
void SendBattlePacket(DWORD vid, BYTE type);
void SendStatePacket(fPoint & pos, float rot, BYTE eFunc, BYTE uArg);
bool SendPacket(int size, void*buffer);
bool SendSequencePacket();
bool SendAddFlyTargetingPacket(DWORD dwTargetVID, float x, float y);
bool SendShootPacket(BYTE uSkill);
bool SendStartFishing(WORD direction);
bool SendStopFishing(BYTE type, float timeLeft);
bool SendPickupItemPacket(DWORD vid);

int getCurrentPhase();
DWORD getMainCharacterVID();
void GlobalToLocalPosition(long& lx, long& ly);
void LocalToGlobalPosition(LONG& rLocalX, LONG& rLocalY);


//Setters
void SetNetClassPointer(void* stackPointer);
void SetSendFunctionPointer(void* p);
void SetSendBattlePacket(void* func);
void SetSendStatePacket(void* func);
void SetGlobalToLocalFunction(void* func);
void SetSendSequenceFunction(void* func);
void SetLocalToGlobalFunction(void* func);
void SetBCheckAdvanceFunction(DetoursHook<tBackground_CheckAdvancing>* hook);
void SetICheckAdvanceFunction(DetoursHook<tInstanceBase_CheckAdvancing>* hook);

//Blocks client from send fishing packets
void SetFishingPacketsBlock(bool val); //1 block packets, 0 do not block packets

//WallHack
void SetBuildingWallHack(bool val); //1 turn on wallhack
void SetMonsterTerrainWallHack(bool val); //1 turn on wallhack






