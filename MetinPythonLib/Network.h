#pragma once
#include <Windows.h>
#include <iostream>
#include "utils.h"


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


namespace PacketHeaders {
	enum {
		HEADER_GC_CHARACTER_ADD = 1,
		HEADER_GC_CHARACTER_DEL = 2,
		HEADER_GC_DEAD = 14,
		HEADER_GC_PHASE = 0xFD,


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

struct MovePlayerPacket {
	BYTE		bFunc;
	BYTE		bArg;
	BYTE		uknown1;
	BYTE		bRot;
	LONG		lX;
	LONG		lY;
	DWORD		dwTime;
	DWORD		uknown2;
};

struct ChangPhasePacket
{
	BYTE phase;
};


struct DeletePlayerPacket
{
	DWORD	dwVID;
};


struct PlayerCreatePacket {

	//DEFAULT
	/*DWORD	dwVID;
#if defined(WJ_SHOW_MOB_INFO)
	DWORD	dwLevel;
	DWORD	dwAIFlag;
#endif
	float	angle;
	long	x;
	long	y;
	long	z;
	BYTE	bType;
	WORD	wRaceNum;
	BYTE	bMovingSpeed;
	BYTE	bAttackSpeed;
	BYTE	bStateFlag;
	DWORD	dwAffectFlag[2];*/
	DWORD dwVID;
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
	DWORD uknown12;

};
#pragma pack(pop)


void logPacket(Packet * packet);

//Hook Functions
DWORD __stdcall __RecvPacket(DWORD return_value, int size, void* buffer);
void __SendPacket(int size, void*buffer);


void SendBattlePacket(DWORD vid, BYTE type);
void SendStatePacket(fPoint & pos, float rot, BYTE eFunc, BYTE uArg);
void SendPacket(int size, void*buffer);
int getCurrentPhase();

void SetNetClassPointer(void* stackPointer);
void SetSendFunctionPointer(void* p);
void SetSendBattlePacket(void* func);
void SetSendStatePacket(void* func);





