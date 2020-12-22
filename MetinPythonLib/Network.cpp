#include "Network.h"
#include "PythonModule.h"
#include "MapCollision.h"
#include <chrono>
#include <ctime>    


//PacketFilter
bool filterInboundOnlyIncluded = false;
bool filterOutboundOnlyIncluded = false;
bool printToConsole = false;
std::set<BYTE> inbound_header_filter;
std::set<BYTE> outbound_header_filter;

using namespace PacketHeaders;
static int gamePhase = 0;
DWORD mainCharacterVID = 0;

typedef bool(__thiscall* tGlobalToLocalPosition)(DWORD classPointer, long& lx, long& ly);
typedef bool(__thiscall* tLocalToGlobalPosition)(DWORD classPointer, LONG& rLocalX, LONG& rLocalY);
typedef bool(__thiscall *tSendPacket)(DWORD classPointer, int size, void* buffer);
typedef bool(__thiscall *tSendAttackPacket)(DWORD classPointer, BYTE type, DWORD vid);
typedef bool(__thiscall *tSendStatePacket)(DWORD classPointer, fPoint& pos, float rot, BYTE eFunc, BYTE uArg);
typedef bool(__thiscall* tSendSequencePacket)(DWORD classPointer);

tLocalToGlobalPosition fLocalToGlobalPosition;
tSendStatePacket fSendStatePacket;
tSendAttackPacket fSendAttackPacket;
tSendPacket fSendPacket;
tSendSequencePacket fSendSequencePacket;
tGlobalToLocalPosition fGlobalToLocalPosition;
DWORD *networkclassPointer;

void printPacket(DWORD calling_function, Packet* p, bool type) {
	if (INBOUND == type) {
		printf("[INBOUND]\n");
	}
	else {
		printf("[OUTBOUND]\n");
	}
	printf("Header: %d\n", p->header);
	printf("Size: %d\n", p->data_size);
	printf("Calling Address: %#x\n", calling_function);
	printf("Content-Bytes: ");
	for (int i = 0; i < p->data_size; i++) {
		printf("%#x ", p->data[i]);
	}
	printf("\n\n");
}

void setFilterMode(PACKET_TYPE t,bool val) {
	if(t == INBOUND)
		filterInboundOnlyIncluded = val;
	else
		filterOutboundOnlyIncluded = val;
}

void clearPacketFilter(PACKET_TYPE t) {
	if (t == INBOUND)
		inbound_header_filter.clear();
	else
		outbound_header_filter.clear();
}

void addHeaderFilter(BYTE header,PACKET_TYPE t) {
	if (t == INBOUND)
		inbound_header_filter.insert(header);
	else
		outbound_header_filter.insert(header);
}

void openConsole()
{
	filterInboundOnlyIncluded = false;
	filterOutboundOnlyIncluded = false;
	clearPacketFilter(OUTBOUND);
	clearPacketFilter(INBOUND);
	AllocConsole();
#ifdef _DEBUG
	setDebugOff();
	return;
#endif
	system("cls");
	SetConsoleTitle("PACKET SNIFFER");
}

void closeConsole()
{
	filterInboundOnlyIncluded = false;
	filterOutboundOnlyIncluded = false;
	stopFilterPacket();
	clearPacketFilter(OUTBOUND);
	clearPacketFilter(INBOUND);
#ifdef _DEBUG
	setDebugOn();
	return;
#endif
	FreeConsole();
}

void startFilterPacket()
{
	printToConsole = true;
}

void stopFilterPacket()
{
	printToConsole = false;
}

void removeHeaderFilter(BYTE header, PACKET_TYPE t) {
	if (t == INBOUND)
		inbound_header_filter.erase(header);
	else 
		outbound_header_filter.erase(header);
}

std::set<BYTE>* getPacketFilter(PACKET_TYPE t) {
	if (t == INBOUND)
		return &inbound_header_filter;
	else
		return &outbound_header_filter;
}

void __stdcall executeScript() {
	executePythonFile((char*)"script.py");
}

bool __stdcall __RecvPacket(DWORD return_function,bool return_value,int size, void* buffer) {
	executeTimerFunctions();
#ifdef USE_INJECTION_RECV_HOOK
	_RecvRoutine();
#endif

	if (return_value != 0 && size > 0 && *(BYTE*)buffer != 0) {
		Packet packet(size, (BYTE*)buffer);

		//PacketFilter
		if (printToConsole) {
			if (inbound_header_filter.find(packet.header) == inbound_header_filter.end()) {
				if(!filterInboundOnlyIncluded)
					printPacket(return_function, &packet, INBOUND);
			}
			else {
				if (filterInboundOnlyIncluded)
					printPacket(return_function, &packet, INBOUND);
			}
		}

		switch (packet.header) {
		case HEADER_GC_CHARACTER_ADD: {
			if (packet.data_size == 0) {
				break;
			}
			PlayerCreatePacket instance;
			fillPacket(&packet, &instance);
			GlobalToLocalPosition(instance.x, instance.y);
			appendNewInstance(instance);
			break;
		}
		case HEADER_GC_CHARACTER_MOVE: {
			if (packet.data_size == 0) {
				break;
			}
			CharacterMovePacket instance;
			fillPacket(&packet, &instance);
			GlobalToLocalPosition(instance.lX, instance.lY);
			changeInstancePosition(instance);
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
			ChangePhasePacket phase;
			fillPacket(&packet, &phase);
			//printf("Phase Change %d\n",phase.phase);
			gamePhase = phase.phase;
			if (phase.phase == PHASE_LOADING) {
				clearInstances();
				freeCurrentMap();
			}
			else if (phase.phase == PHASE_GAME) {
				setCurrentCollisionMap();
				static bool hasPassed = 0;
				if (!hasPassed) {
					//setTimerFunction(executeScript, 2);
					executeScript();
					hasPassed = true;
					/*static auto start = std::chrono::system_clock::now();
					auto end = std::chrono::system_clock::now();
					auto elapsed_seconds = end - start;

					if (elapsed_seconds.count() > 4) {
						executePythonFile((char*)"script.py");
						hasPassed = true;
					}*/
				}
			}
			break;
		}

		case HEADER_GC_MAIN_CHARACTER:{
			if (PHASE_LOADING == gamePhase) {
				MainCharacterPacket m;
				fillPacket(&packet, &m);
				DEBUG_INFO_LEVEL_2("MAIN VID: %d", m.dwVID);
				mainCharacterVID = m.dwVID;
			}
			break;
		}

		case HEADER_GC_DEAD: {
			DeadPacket dead;
			fillPacket(&packet, &dead);
			changeInstanceIsDead(dead.vid, 1);
			break;
		}
		}
	}



	return return_value;

}

void SendBattlePacket(DWORD vid, BYTE type)
{
	fSendAttackPacket(*networkclassPointer, type, vid);
}

void SendStatePacket(fPoint & pos, float rot, BYTE eFunc, BYTE uArg)
{
	fSendStatePacket(*networkclassPointer, pos, rot, eFunc, uArg);
}

bool SendPacket(int size, void*buffer) {
	return fSendPacket(*networkclassPointer, size, buffer);
	//Packet packet(size, (BYTE*)buffer);
	//logPacket(&packet);
}

bool SendSequencePacket()
{
	return fSendSequencePacket(*networkclassPointer);
}

bool SendAddFlyTargetingPacket(DWORD dwTargetVID, float x, float y)
{
	AddFlyTargetingPacket packet;
	packet.dwTargetVID = dwTargetVID;
	packet.lX = (long)x;
	packet.lY = (long)y;
	LocalToGlobalPosition(packet.lX, packet.lY);

	if (SendPacket(sizeof(AddFlyTargetingPacket), &packet))
		return SendSequencePacket();
	return false;
}
bool SendShootPacket(BYTE uSkill)
{
	ShootPacket packet;
	packet.type = uSkill;

	if (SendPacket(sizeof(ShootPacket), &packet))
		return SendSequencePacket();
	return false;
}

bool SendStartFishing(WORD direction)
{
	StartFishing packet;
	packet.direction = direction;
	if (SendPacket(sizeof(StartFishing), &packet))
		return SendSequencePacket();

	return false;

}

bool SendStopFishing(BYTE type, float timeLeft)
{
	StopFishing packet;
	packet.type = type;
	packet.timeLeft = timeLeft;
	if (SendPacket(sizeof(StopFishing), &packet))
		return SendSequencePacket();

	return false;
}

void GlobalToLocalPosition(long& lx, long& ly)
{
	fGlobalToLocalPosition(*networkclassPointer, lx,ly);
}

void LocalToGlobalPosition(LONG& rLocalX, LONG& rLocalY)
{
	fLocalToGlobalPosition(*networkclassPointer, rLocalX, rLocalY);
}


void __SendPacket(void* retAddress,int size, void*buffer){
	Packet packet(size, (BYTE*)buffer);
	//PacketFilter
	if (printToConsole) {
		if (outbound_header_filter.find(packet.header) == outbound_header_filter.end()) {
			if (!filterOutboundOnlyIncluded)
				printPacket((DWORD)retAddress, &packet, OUTBOUND);
		}
		else {
			if (filterOutboundOnlyIncluded)
				printPacket((DWORD)retAddress, &packet, OUTBOUND);
		}
	}

	switch (packet.header) {
	case HEADER_CG_CHARACTER_MOVE: {
		CharacterStatePacket move;
		fillPacket(&packet, &move);

		break;
	}
	}
	return;
}

int getCurrentPhase()
{
	return gamePhase;
}

DWORD getMainCharacterVID()
{
	return mainCharacterVID;
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

void SetGlobalToLocalFunction(void* func)
{
	fGlobalToLocalPosition = (tGlobalToLocalPosition)func;
}


void SetSendSequenceFunction(void* func)
{
	fSendSequencePacket = (tSendSequencePacket)func;
}

void SetLocalToGlobalFunction(void* func)
{
	fLocalToGlobalPosition = (tLocalToGlobalPosition)func;
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
