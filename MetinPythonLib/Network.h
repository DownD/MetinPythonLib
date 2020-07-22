#pragma once
#include <Windows.h>
#include <iostream>

enum {
	PHASE_LOADING = 4,
	PHASE_CHARACTER_CHOSE = 2,
	PHASE_GAME = 5
};


namespace PacketHeaders {
	enum {
		HEADER_GC_CHARACTER_ADD = 1,
		HEADER_GC_CHARACTER_DEL = 2,
		HEADER_GC_PHASE = 0xFD
	};
}


struct Packet {
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
DWORD __stdcall __RecvPacket(DWORD return_value, int size, void* buffer);
void SendPacket(int size, void*buffer);
void __SendPacket(int size, void*buffer);

int getCurrentPhase();





