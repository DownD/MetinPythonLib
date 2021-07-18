#include "Network.h"
#include "PythonModule.h"
#include "MapCollision.h"
#include <chrono>
#include <ctime>

#define MAX_TELEPORT_DIST 2400


//PacketFilter
bool filterInboundOnlyIncluded = false;
bool filterOutboundOnlyIncluded = false;
bool printToConsole = false;
std::set<BYTE> inbound_header_filter;
std::set<BYTE> outbound_header_filter;

using namespace PacketHeaders;
int gamePhase = 0;
DWORD mainCharacterVID = 0;

typedef bool(__thiscall* tGlobalToLocalPosition)(DWORD classPointer, long& lx, long& ly);
typedef bool(__thiscall* tLocalToGlobalPosition)(DWORD classPointer, LONG& rLocalX, LONG& rLocalY);
typedef bool(__thiscall *tSendAttackPacket)(DWORD classPointer, BYTE type, DWORD vid);
typedef bool(__thiscall* tSendSequencePacket)(DWORD classPointer);

tLocalToGlobalPosition fLocalToGlobalPosition;
tSendAttackPacket fSendAttackPacket;
tSendPacket fSendPacket;
tSendSequencePacket fSendSequencePacket;
tGlobalToLocalPosition fGlobalToLocalPosition;

//enables packet fixer for speedHack to work
#define PACKET_NUM_TO_SEND_WALK 5 //Number of walk packets recived to send a real one

//For boosting
fPoint lastPoint = { 0,0 };
bool lastPointIsStored = false;
float speed_Multiplier = 1.0;

PyObject* netMod;//Leaks
DWORD *networkclassPointer;

void setPhase(ChangePhasePacket& phase);

//For wallhack
/*
WALLHACK
Note: it is possible to hook 1 more function and have wllhack divided for terrain, monsters and buildings by using
__TestObjectCollision,TestActorCollision, for buildings and monsters
,
*/
DetoursHook<tBackground_CheckAdvancing>* backGroundCheckAdvanceHook = 0;
DetoursHook<tInstanceBase_CheckAdvancing>* instanceBaseCheckAdvanceHook = 0;
DetoursHook<tSendStatePacket>* sendStatePacketHook = 0;

bool wallHackBuildings = 0;
bool wallHackTerrainMonsters = 0;

//If true, fishing packets will be blocked
bool blockFishingPackets = 0;
bool block_next_sequence = 0;


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

bool RecvLoadingPhase(Packet& packet, int size) {
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
	case HEADER_GC_MAIN_CHARACTER: {
		MainCharacterPacket m;
		fillPacket(&packet, &m);
		DEBUG_INFO_LEVEL_2("MAIN VID: %d", m.dwVID);
		mainCharacterVID = m.dwVID;
		break;
	}
	case HEADER_GC_PHASE: {
		ChangePhasePacket phase;
		fillPacket(&packet, &phase);
		if (phase.phase != 1 && phase.phase != 2)
			setPhase(phase);
		break;
	}
	}

	return true;
}

bool RecvGamePhase(Packet & packet, int size) {
	switch (packet.header) {
	case HEADER_GC_FISHING: {
		if (blockFishingPackets) {
			if (packet.data_size >= 1 && (packet.data[0] == 0x0 || packet.data[0] == 0x4)) { //This is for refresh the inventory window
				return false;
			}
		}
		break;
	}
	case HEADER_GC_ITEM_GROUND_ADD: {
		GroundItemAddPacket instance;
		fillPacket(&packet, &instance);
		GlobalToLocalPosition(instance.x, instance.y);
		addItemGround(instance);
		break;
	}
	case HEADER_GC_ITEM_GROUND_DEL: {
		GroundItemDeletePacket instance;
		fillPacket(&packet, &instance);
		delItemGround(instance);
		break;
	}
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
		if(phase.phase != 1 && phase.phase != 2)
			setPhase(phase);
		break;
	}
	case HEADER_CG_DIG_MOTION: {
		if (packet.data_size == 0) {
			break;
		}
		PacketDigMotion instance_;
		fillPacket(&packet, &instance_);
		callDigMotionCallback(instance_.vid, instance_.target_vid, instance_.count);
		break;
	}

	case HEADER_GC_DEAD: {
		DeadPacket dead;
		fillPacket(&packet, &dead);
		changeInstanceIsDead(dead.vid, 1);
		break;
	}
	}

	return true;
}

void setPhase(ChangePhasePacket& phase) {
	if (phase.phase > 5)
		return;
	//printf("Phase Packet %d\n",phase.phase);
	//if (phase.phase == 1 || phase.phase == 10 || phase.phase == 2) //The server sends 2 strange packets values(1,10), that disrupt the fase check
	//	return;
	gamePhase = phase.phase;
	lastPointIsStored = false;
	DEBUG_INFO_LEVEL_2("Phased changed to: %d", gamePhase);
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
}



bool __stdcall __RecvPacket(DWORD return_function,bool return_value,int size, void* buffer) {
	//executeTimerFunctions();
#ifdef USE_INJECTION_RECV_HOOK
	_RecvRoutine();
#endif
	DEBUG_INFO_LEVEL_4("Hook RecvPacket called return size=%d, return_val=%d", size, return_value);
	if (return_value != 0 && size > 0 && *(BYTE*)buffer != 0) {
		Packet packet(size, (BYTE*)buffer);

		DEBUG_INFO_LEVEL_4("Hook RecvPacket header=%d", packet.header);

		//PacketFilter
		if (printToConsole) {
			if (inbound_header_filter.find(packet.header) == inbound_header_filter.end()) {
				if (!filterInboundOnlyIncluded)
					printPacket(return_function, &packet, INBOUND);
			}
			else {
				if (filterInboundOnlyIncluded)
					printPacket(return_function, &packet, INBOUND);
			}
		}


		switch (gamePhase) {
		case PHASE_GAME:
			return_value = RecvGamePhase(packet, size);
			break;

		case PHASE_LOADING:
			return_value = RecvLoadingPhase(packet, size);
			break;
		default: {
			switch (packet.header) {
			case HEADER_GC_PHASE: {
				ChangePhasePacket phase;
				fillPacket(&packet, &phase);
				setPhase(phase);
				break;
			}
			}
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
	sendStatePacketHook->originalFunction(*networkclassPointer,pos,rot,eFunc,uArg);
}

bool SendPacket(int size, void*buffer) {
	//Packet packet(size, (BYTE*)buffer);
	bool val = fSendPacket(*networkclassPointer, size, buffer);
	//printPacket((DWORD)val, &packet, OUTBOUND);
	return val;
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

bool SendPickupItemPacket(DWORD vid)
{
	PickupPacket packet;
	packet.vid = vid;
	DEBUG_INFO_LEVEL_4("Send Pickup Item vid=%d", vid);
	if (SendPacket(sizeof(PickupPacket), &packet))
		return SendSequencePacket();

	return false;
}

bool SendUseSkillPacket(DWORD dwSkillIndex, DWORD dwTargetVID)
{
	SkillPacket packet;
	packet.vid = dwTargetVID;
	packet.dwSkillIndex = dwSkillIndex;
	DEBUG_INFO_LEVEL_3("Sending Skill Packet index=%d, vid =%d", dwSkillIndex, dwTargetVID);
	if (SendPacket(sizeof(SkillPacket), &packet))
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

void SetSpeedMultiplier(float val)
{
	speed_Multiplier = val;
}


bool __stdcall __SendPacket(DWORD classPointer,DetoursHook<tSendPacket>* hook, void* retAddress,int size, void*buffer){
	DEBUG_INFO_LEVEL_4("Hook SendPacket called return size=%d", size);
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
		//case 77:
		case HEADER_CG_FISHING: {
			if (blockFishingPackets) {
				DEBUG_INFO_LEVEL_2("Blocking Fishing Packet");
				block_next_sequence = 1; //Also block sequence packet
				return 1;			//Block fishing packets
			}
			break;
		}
	}
	return hook->originalFunction((DWORD)classPointer,size,buffer);
}

bool __fastcall __SendSequencePacket(DWORD classPointer)
{
	if (block_next_sequence) {
		block_next_sequence = 0;
		DEBUG_INFO_LEVEL_4("Hook SendSequence called return val=%d",1);
		return 1;
	}
	bool val= fSendSequencePacket(classPointer);
	DEBUG_INFO_LEVEL_4("Hook SendSequence called return val=%d", val);
	return val;
}

//return 0 to ignore collisions
bool __fastcall __BackgroundCheckAdvanced(DWORD classPointer, DWORD EDX, void* instanceBase)
{
	//DEBUG_INFO_LEVEL_4("Hook __BackgroundCheckAdvanced called");
	if (wallHackBuildings)
		return false;
	else
		return backGroundCheckAdvanceHook->originalFunction(classPointer,instanceBase);
}

//return 0 to ignore collisions
bool __fastcall __InstanceBaseCheckAdvanced(DWORD classPointer)
{
	//DEBUG_INFO_LEVEL_4("Hook __InstanceBaseCheckAdvanced called");
	if (wallHackTerrainMonsters)
		return false;
	else
		return instanceBaseCheckAdvanceHook->originalFunction(classPointer);
}

bool __fastcall __SendStatePacket(DWORD classPointer, DWORD EDX, fPoint& pos, float rot, BYTE eFunc, BYTE uArg)
{
	if (pos.x < 0 || pos.y < 0) {
		return sendStatePacketHook->originalFunction(classPointer, pos, rot, eFunc, uArg);
	}

	if (eFunc == CHAR_STATE_FUNC_WALK && speed_Multiplier>1) {
		if (!lastPointIsStored) {
			lastPoint = pos;
			lastPointIsStored = true;
		}
		else {
			//DEBUG_INFO_LEVEL_3("starting X->%f, Y->%f, EndPos X->%f, Y->%f", lastPoint.x, lastPoint.y, pos.x, pos.y);
			if (distance(lastPoint.x, lastPoint.y, pos.x, pos.y) > 1) {

				//DEBUG_INFO_LEVEL_3("BOSTING");
				fPoint currPosition = { 0,0 };
				if (getLastMovementType() == MOVE_POSITION)
					currPosition = getPointAtDistanceTimes(lastPoint.x, lastPoint.y, pos.x, pos.y, speed_Multiplier / 2);
				else
					currPosition = getPointAtDistanceTimes(lastPoint.x, lastPoint.y, pos.x, pos.y, speed_Multiplier);


				//Check if it is destination position and if it is in between the points
				if (getLastMovementType() == MOVE_POSITION) {

					fPoint destPos = getLastDestPosition();
					if (checkPointBetween(lastPoint.x, lastPoint.y, destPos.x, destPos.y, currPosition.x, currPosition.y)) {
						//DEBUG_INFO_LEVEL_3("CharacterState Point is in between");
						currPosition.x = destPos.x;
						currPosition.y = destPos.y;
					}
				}
				float dist = distance(lastPoint.x, lastPoint.y, currPosition.x, currPosition.y);
				int steps = dist / MAX_TELEPORT_DIST;


				//DEBUG_INFO_LEVEL_3("StartPos X->%f, Y->%f, EndPos X->%f, Y->%f, dist %f", lastPoint.x, lastPoint.y, currPosition.x, currPosition.y, dist);

				//Fix large movement speed
				for (int step = 0; step < steps; step++) {
					fPoint this_pos = getPointAtDistanceTimes(lastPoint.x, lastPoint.y, currPosition.x, currPosition.y, (float)(step + 1) / (float)(steps + 1));
					pos.x = this_pos.x;
					pos.y = this_pos.y;

					//DEBUG_INFO_LEVEL_3("CharStateBoostedSteps X->%f, Y->%f, eFunc %d, rot %f, uArg %d, step %d, dist %f steps %f", pos.x, pos.y, 0, rot, uArg, step, distance(lastPoint.x, lastPoint.y, pos.x, pos.y), (float)(step + 1) / (float)(steps + 1));
					sendStatePacketHook->originalFunction(classPointer, pos, rot, CHAR_STATE_FUNC_STOP, 0);
				}


				pos.x = currPosition.x;
				pos.y = currPosition.y;
				//DEBUG_INFO_LEVEL_3("CharStateBoosted X->%f, Y->%f, eFunc %d, rot %f, uArg %d, dist %f", pos.x, pos.y, 0, rot, uArg, distance(lastPoint.x, lastPoint.y, pos.x, pos.y));

				sendStatePacketHook->originalFunction(classPointer, pos, rot, CHAR_STATE_FUNC_STOP, 0);

				//Reload Mobs
				pos.x += 200;
				pos.y -= 200;
				sendStatePacketHook->originalFunction(classPointer, pos, rot, eFunc, uArg);

				pos.x -= 200;
				pos.y += 200;
				sendStatePacketHook->originalFunction(classPointer, pos, rot, eFunc, uArg);

				pos.x = currPosition.x;
				pos.y = currPosition.y;

				setPixelPosition(pos);
				lastPoint = pos;
			}
		}


	}
	else {
		lastPointIsStored = false;
	}
	DEBUG_INFO_LEVEL_3("CharStateNormal X->%f, Y->%f, eFunc %d, rot %f, uArg %d", pos.x, pos.y, eFunc, rot, uArg);
	return sendStatePacketHook->originalFunction(classPointer, pos, rot, eFunc, uArg);
}

int getCurrentPhase()
{
	return gamePhase;
}

DWORD getMainCharacterVID()
{
	if (netMod == 0) {
		LoadPythonNetModule();
	}
	//return mainCharacterVID;
	PyObject* poArgs = Py_BuildValue("()");
	long ret = 0;

	if (netMod == 0) {
		return 0;
	}

	if (PyCallClassMemberFunc(netMod, "GetMainActorVID", poArgs, &ret)) {
		Py_DECREF(poArgs);
		return ret;
	}

	Py_DECREF(poArgs);
	return ret;
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

void SetSendStatePacket(DetoursHook<tSendStatePacket>* hook)
{
	sendStatePacketHook = hook;
	sendStatePacketHook->HookFunction();
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

void SetBCheckAdvanceFunction(DetoursHook<tBackground_CheckAdvancing>* hook)
{
	backGroundCheckAdvanceHook = hook;
	backGroundCheckAdvanceHook->HookFunction();
}

void SetICheckAdvanceFunction(DetoursHook<tInstanceBase_CheckAdvancing>* hook)
{
	instanceBaseCheckAdvanceHook = hook;
	instanceBaseCheckAdvanceHook->HookFunction();
}



void SetFishingPacketsBlock(bool val)
{
	blockFishingPackets = val;
}

void SetBuildingWallHack(bool val)
{
	wallHackBuildings = val;
}

void SetMonsterTerrainWallHack(bool val)
{
	wallHackTerrainMonsters = val;
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

void LoadPythonNetModule()
{
	netMod = PyImport_ImportModule("net");

	if(!netMod){
		DEBUG_INFO_LEVEL_1("Could not import net module! Maybe init.py was not executed");
	}
	if (netMod) {
		DEBUG_INFO_LEVEL_1("Net module loaded");
	}
}
