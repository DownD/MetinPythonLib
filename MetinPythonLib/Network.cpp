#include "Network.h"
#include "PythonModule.h"
#include "MapCollision.h"

using namespace PacketHeaders;

static int gamePhase = 0;

typedef bool(__thiscall *tSendPacket)(DWORD classPointer, int size, void* buffer);

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
			PyObject* vid = PyLong_FromLong(instance.dwVID);
			PyDict_SetItem(instanceList, vid, vid);
			break;
		}
		case HEADER_GC_CHARACTER_DEL: {
			if (packet.data_size == 0) {
				break;
			}
			DeletePlayerPacket instance_;
			fillPacket(&packet, &instance_);
			PyObject* vid = PyLong_FromLong(instance_.dwVID);
			PyDict_DelItem(instanceList, vid);
			break;
		}
		case HEADER_GC_PHASE: {
			ChangPhasePacket phase;
			fillPacket(&packet, &phase);
			gamePhase = phase.phase;
			if (phase.phase == PHASE_LOADING) {
				PyDict_Clear(instanceList);
				freeCurrentMap();
			}
			else if (phase.phase == PHASE_GAME) {
				setCurrentCollisionMap();
			}
			break;

		}
		}
	}
	return return_value;

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
