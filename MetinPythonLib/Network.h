#pragma once
#include <Windows.h>
#include <iostream>
#include "utils.h"
#include <set>


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
};

enum {
	PHASE_LOADING = 4,
	PHASE_CHARACTER_CHOSE = 2,
	PHASE_GAME = 5
};

//85 - Char Move
//84 -  Remove?
//125 - Add?
//82 chat
//95 255 7
//Dead 
//24, firstbyte appears to be 1, size of 9
//124 size of 9
namespace PacketHeaders {
	enum {
		HEADER_GC_CHARACTER_ADD = 125,
		HEADER_GC_CHARACTER_DEL = 84,
		HEADER_GC_DEAD = 127,
		HEADER_GC_PHASE = 255,
		HEADER_GC_CHARACTER_MOVE = 85,


		//To server
		HEADER_CG_SEND_CHAT = 3,
		HEADER_CG_CHARACTER_MOVE = 7
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
	int size = min(p->data_size, sizeof(T));
	memcpy(_struct, p->data, size);
}

struct DeadPacket
{
	DWORD	vid;
};

struct AttackPacket
{
	BYTE	header;
	BYTE	bType;			
	DWORD	dwVictimVID;
	BYTE	bCRCMagicCubeProcPiece;
	BYTE	bCRCMagicCubeFilePiece;
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
	//DWORD	dwLevel;
	//DWORD	dwAIFlag;
	float	angle;
	long	x;
	long	y;
	long	z;
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
void __SendPacket(int size, void*buffer);


void SendBattlePacket(DWORD vid, BYTE type);
void SendStatePacket(fPoint & pos, float rot, BYTE eFunc, BYTE uArg);
void SendPacket(int size, void*buffer);
void GlobalToLocalPosition(long& lx, long& ly);
int getCurrentPhase();

void SetNetClassPointer(void* stackPointer);
void SetSendFunctionPointer(void* p);
void SetSendBattlePacket(void* func);
void SetSendStatePacket(void* func);
void SetGlobalToLocalPacket(void* func);





