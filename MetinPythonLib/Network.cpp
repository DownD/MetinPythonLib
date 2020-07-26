#include "Network.h"
#include "PythonModule.h"
#include "MapCollision.h"

using namespace PacketHeaders;

static int gamePhase = 0;

typedef bool(__thiscall *tSendPacket)(DWORD classPointer, int size, void* buffer);
typedef bool(__thiscall *tSendAttackPacket)(DWORD classPointer, DWORD arg0,BYTE type, DWORD vid, DWORD arg1);
typedef bool(__thiscall *tSendStatePacket)(DWORD classPointer, fPoint& pos, float rot, BYTE eFunc, BYTE uArg, BYTE arg0);

tSendStatePacket fSendStatePacket;
tSendAttackPacket fSendAttackPacket;
tSendPacket fSendPacket;
DWORD *networkclassPointer;

void logPacket(Packet * packet) {
	printf("Header: %d\n", packet->header);
	printf("Content-Bytes: ");
	for (int i = 0; i < packet->data_size; i++) {
		printf("%#x ", packet->data[i]);
	}
	printf("\n\n");

}

DWORD __stdcall __RecvPacket(DWORD return_value,int size, void* buffer) {
	if (return_value != 0 && size > 0 && *(BYTE*)buffer != 0) {
		Packet packet(size, (BYTE*)buffer);
#ifdef _DEBUG
		//logPacket(&packet);
#endif
		switch (packet.header) {
		case HEADER_GC_CHARACTER_ADD: {
			if (packet.data_size == 0) {
				break;
			}
			PlayerCreatePacket instance;
			fillPacket(&packet, &instance);
			appendNewInstance(instance.dwVID);
			break;
		}
		case HEADER_GC_CHARACTER_DEL: {
			if (packet.data_size == 0) {
				break;
			}
			DeletePlayerPacket instance_;
			fillPacket(&packet, &instance_);
			deleteInstance(instance_.dwVID);
			break;
		}
		case HEADER_GC_PHASE: {
			ChangPhasePacket phase;
			fillPacket(&packet, &phase);
			gamePhase = phase.phase;
			if (phase.phase == PHASE_LOADING) {
				clearInstances();
				freeCurrentMap();
			}
			else if (phase.phase == PHASE_GAME) {
				setCurrentCollisionMap();
			}
			break;

		}
		case HEADER_GC_DEAD: {
			DeadPacket dead;
			fillPacket(&packet, &dead);
			changeInstanceIsDead(dead.vid, 1);
		}
		}
	}
	return return_value;

}

void SendBattlePacket(DWORD vid, BYTE type)
{
	fSendAttackPacket(*networkclassPointer, 0, type, vid, 0);
}

void SendStatePacket(fPoint & pos, float rot, BYTE eFunc, BYTE uArg)
{
	fSendStatePacket(*networkclassPointer, pos, rot, eFunc, uArg,0);
}

void SendPacket(int size, void*buffer) {
	fSendPacket(*networkclassPointer, size, buffer);
	//Packet packet(size, (BYTE*)buffer);
	//logPacket(&packet);
}


void __SendPacket(int size, void*buffer){
	Packet packet(size, (BYTE*)buffer);
#ifdef _DEBUG
	//logPacket(&packet);
#endif

	switch (packet.header) {
		case HEADER_CG_CHARACTER_MOVE: {
			MovePlayerPacket move;
			fillPacket(&packet, &move);
			#ifdef _DEBUG
			//printf("bFunc=%d bArg=%d u1=%d bRot=%d lX=%d lY=%d time=%d u2=%d\n",move.bFunc,move.bArg, move.uknown1,move.bRot,move.lX,move.lY,move.uknown2 );
			#endif
		}
	}
}

int getCurrentPhase()
{
	return gamePhase;
}

void SetNetClassPointer(void * stackPointer)
{
	networkclassPointer = (DWORD*)stackPointer;
}

void SetSendFunctionPointer(void * p)
{
	fSendPacket = (tSendPacket)p;
}

void SetSendBattlePacket(void * func)
{
	fSendAttackPacket = (tSendAttackPacket)func;
}

void SetSendStatePacket(void * func)
{
	fSendStatePacket = (tSendStatePacket)func;
}

Packet::Packet(int size, void * buffer)
{
	if (size > 1) {
		header = *(BYTE*)buffer;
		data_size = size - 1;
		data = (BYTE*)((int)buffer + 1);
	}
	else {
		header = 0;
		data_size = 0;
		data = 0;
	}
}
